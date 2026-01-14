"""
Script to process files, mark @ServerImpl annotations as processed, and generate registration code.

Takes a list of files and library_dir as arguments.
For each file:
1. Checks if it has @ServerImpl annotation
2. Marks it as processed (replaces /* @ServerImpl(...) */ with /*--@ServerImpl(...)--*/) if found
3. Extracts class name and @ServerImpl content

Then generates RegisterServer calls and updates ServerProviderInit.h
"""

import sys
import re
from pathlib import Path


def extract_bracket_content(content):
    """Extract content from brackets, removing quotes if present."""
    if not content:
        return ""
    # Strip whitespace
    content = content.strip()
    # Remove quotes if present (handles both single and double quotes)
    if (content.startswith('"') and content.endswith('"')) or \
       (content.startswith("'") and content.endswith("'")):
        content = content[1:-1]
    return content


def check_and_comment_server_impl(file_path):
    """
    Check if a file contains @ServerImpl annotation and mark it as processed.
    
    Args:
        file_path: Path to the file to process
    
    Returns:
        dict: Dictionary with 'found' (bool), 'matches' (list), 'modified' (bool), and 'error' (str if any)
    """
    result = {
        'found': False,
        'matches': [],
        'modified': False,
        'error': None
    }
    
    try:
        file_path = Path(file_path)
        if not file_path.exists():
            result['error'] = f"File does not exist: {file_path}"
            return result
        
        if not file_path.is_file():
            result['error'] = f"Path is not a file: {file_path}"
            return result
        
        # Read the file content
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            lines = f.readlines()
        
        # Pattern to match @ServerImpl annotation (search for /* @ServerImpl("content") */ or /*@ServerImpl("content")*/)
        # Also check for already processed /*--@ServerImpl("content")--*/ pattern
        server_impl_annotation_pattern = re.compile(r'(\s*)/\*\s*@ServerImpl\s*\(([^)]*)\)\s*\*/')
        server_impl_processed_pattern = re.compile(r'/\*--\s*@ServerImpl\s*\([^)]*\)\s*--\*/')
        class_pattern = re.compile(r'class\s+(\w+)(?:\s+final)?\s*:\s*public\s+IServer')
        
        modified_lines = lines.copy()
        
        # Check each line for @ServerImpl annotation
        for i, line in enumerate(lines):
            stripped_line = line.strip()
            
            # Skip already processed annotations
            if server_impl_processed_pattern.search(stripped_line):
                continue
            
            # Check if current line has @ServerImpl annotation
            server_impl_match = server_impl_annotation_pattern.search(stripped_line)
            if server_impl_match:
                # Check if this @ServerImpl is followed by a class that inherits from IServer
                # Look ahead up to 5 lines (to handle comments or blank lines)
                found_class = False
                class_name = None
                for j in range(i + 1, min(i + 6, len(lines))):
                    class_match = class_pattern.search(lines[j])
                    if class_match:
                        found_class = True
                        class_name = class_match.group(1)
                        break
                
                # Only process if it's followed by a matching class
                if found_class:
                    # Extract the content inside brackets
                    bracket_content = server_impl_match.group(2)  # Content inside parentheses
                    extracted_content = extract_bracket_content(bracket_content)
                    
                    # Check if already processed (/*--@ServerImpl(...)--*/)
                    if not server_impl_processed_pattern.search(stripped_line):
                        # Preserve indentation and replace with processed marker
                        indent = server_impl_match.group(1)
                        content_part = bracket_content  # Content inside parentheses
                        processed_line = f"{indent}/*--@ServerImpl({content_part})--*/\n"
                        modified_lines[i] = processed_line
                        result['modified'] = True
                    
                    # Store match information
                    # Resolve to full absolute path
                    full_file_path = file_path.resolve()
                    match_info = {
                        'line_number': i + 1,
                        'class_name': class_name,
                        'server_impl_content': extracted_content,
                        'file_path': str(full_file_path)  # Full absolute path
                    }
                    result['matches'].append(match_info)
                    result['found'] = True
        
        # Write the modified content if changes were made
        if result['modified']:
            with open(file_path, 'w', encoding='utf-8', errors='ignore') as f:
                f.writelines(modified_lines)
    
    except Exception as e:
        result['error'] = f"Error processing file: {str(e)}"
    
    return result


def generate_include_statements(registrations, library_dir):
    """
    Generate #include statements for all class files using full absolute paths.
    
    Args:
        registrations: List of dicts with 'file_path'
        library_dir: Path to the library directory (not used, kept for compatibility)
    
    Returns:
        str: Generated #include statements with full absolute paths
    """
    if not registrations:
        return ""
    
    includes = []
    seen_paths = set()
    
    for reg in registrations:
        file_path = Path(reg['file_path']).resolve()
        
        # Skip if we've already included this file
        if str(file_path) in seen_paths:
            continue
        seen_paths.add(str(file_path))
        
        # Use full absolute path
        include_path = f'"{file_path}"'
        includes.append(f'#include {include_path}')
    
    return '\n'.join(includes)


def generate_registration_code(registrations):
    """
    Generate C++ code snippet for RegisterServer calls.
    
    Args:
        registrations: List of dicts with 'class_name' and 'server_impl_content'
    
    Returns:
        str: Generated C++ code
    """
    if not registrations:
        return "    return false;"
    
    lines = []
    for reg in registrations:
        class_name = reg['class_name']
        content = reg['server_impl_content']
        # Put content in quotes
        quoted_content = f'"{content}"'
        lines.append(f'    ServerProvider::RegisterServer<{class_name}>({quoted_content});')
    
    lines.append('    return true;')
    return '\n'.join(lines)


def update_server_factory_init(library_dir, registration_code, include_statements):
    """
    Update ServerProviderInit.h with the generated include statements and registration code.
    
    Args:
        library_dir: Path to the library directory
        registration_code: The generated C++ code to insert
        include_statements: The generated #include statements
    
    Returns:
        dict: Dictionary with 'success' (bool) and 'error' (str if any)
    """
    result = {
        'success': False,
        'error': None
    }
    
    try:
        library_dir = Path(library_dir)
        init_file = library_dir / "include" / "ServerProviderInit.h"
        
        if not init_file.exists():
            result['error'] = f"ServerProviderInit.h not found at {init_file}"
            return result
        
        # Read the file
        with open(init_file, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
        
        # Pattern to match the placeholder Init() function
        # Match: inline Bool Init() { ... return false; ... }
        pattern = re.compile(
            r'(inline\s+Bool\s+Init\s*\(\s*\)\s*\{)\s*return\s+false\s*;(\s*\})',
            re.MULTILINE | re.DOTALL
        )
        
        # Replace the placeholder with includes + registration code
        # First, add includes before the Init() function
        if include_statements:
            # Find the position of the Init() function
            init_match = pattern.search(content)
            if init_match:
                # Find the line before Init() function (usually a comment or blank line)
                init_start = init_match.start()
                # Look backwards to find where to insert includes (after last #include or after comment block)
                # Find the last #include or /** comment
                last_include_pos = content.rfind('#include', 0, init_start)
                last_comment_pos = content.rfind('/**', 0, init_start)
                
                # Insert after the last #include or after the comment block
                if last_include_pos != -1:
                    # Find the end of the last #include line
                    insert_pos = content.find('\n', last_include_pos) + 1
                    # Skip any blank lines
                    while insert_pos < init_start and content[insert_pos:insert_pos+1].strip() == '':
                        insert_pos += 1
                    content = content[:insert_pos] + include_statements + '\n\n' + content[insert_pos:]
                elif last_comment_pos != -1:
                    # Find the end of the comment block (look for */)
                    comment_end = content.find('*/', last_comment_pos) + 2
                    insert_pos = content.find('\n', comment_end) + 1
                    # Skip any blank lines
                    while insert_pos < init_start and content[insert_pos:insert_pos+1].strip() == '':
                        insert_pos += 1
                    content = content[:insert_pos] + include_statements + '\n\n' + content[insert_pos:]
                else:
                    # Insert right before Init() function
                    content = content[:init_start] + include_statements + '\n\n' + content[init_start:]
        
        # Now replace the Init() function body
        replacement = r'\1\n' + registration_code + r'\2'
        new_content = pattern.sub(replacement, content)
        
        if new_content == content and not include_statements:
            result['error'] = "Could not find placeholder Init() function to replace"
            return result
        
        # Write the updated content
        with open(init_file, 'w', encoding='utf-8', errors='ignore') as f:
            f.write(new_content)
        
        result['success'] = True
    
    except Exception as e:
        result['error'] = f"Error updating ServerProviderInit.h: {str(e)}"
    
    return result


def main():
    """Main function to run the script from command line."""
    if len(sys.argv) < 3:
        # print("Usage: python L3_process_and_register.py <library_dir> <file1> [file2] [file3] ...")
        # print("Example: python L3_process_and_register.py /path/to/lib /path/to/file1.h /path/to/file2.h")
        sys.exit(1)
    
    library_dir = sys.argv[1]
    file_paths = sys.argv[2:]
    
    # print(f"Processing {len(file_paths)} file(s)...")
    # print(f"Library directory: {library_dir}\n")
    
    all_registrations = []
    processed_count = 0
    commented_count = 0
    
    # Process each file
    for file_path in file_paths:
        # print(f"Processing: {file_path}")
        result = check_and_comment_server_impl(file_path)
        
        if result['error']:
            # print(f"  Error: {result['error']}")
            continue
        
        if result['found']:
            processed_count += 1
            if result['modified']:
                commented_count += 1
                # print(f"  ✓ Marked {len(result['matches'])} @ServerImpl annotation(s) as processed")
            else:
                # print(f"  ✓ Found {len(result['matches'])} @ServerImpl annotation(s) (already processed)")
                pass
            
            # Add registrations to the list
            for match in result['matches']:
                all_registrations.append({
                    'class_name': match['class_name'],
                    'server_impl_content': match['server_impl_content'],
                    'file_path': match.get('file_path', '')  # Include file path
                })
                # print(f"    - Class: {match['class_name']}, @ServerImpl: \"{match['server_impl_content']}\"")
        else:
            # print(f"  - No @ServerImpl annotation found")
            pass
    
    # print(f"\n{'=' * 80}")
    # print(f"Summary:")
    # print(f"  Files processed: {processed_count}/{len(file_paths)}")
    # print(f"  Files with annotations processed: {commented_count}")
    # print(f"  Total registrations: {len(all_registrations)}")
    # print(f"{'=' * 80}\n")
    
    if not all_registrations:
        # print("No @ServerImpl annotations found. Nothing to register.")
        sys.exit(0)
    
    # Generate include statements
    # print("Generating include statements...")
    include_statements = generate_include_statements(all_registrations, library_dir)
    if include_statements:
        # print("Generated includes:")
        # print("-" * 80)
        # print(include_statements)
        # print("-" * 80)
        pass
    
    # Generate registration code
    # print("\nGenerating registration code...")
    registration_code = generate_registration_code(all_registrations)
    # print("Generated code:")
    # print("-" * 80)
    # print(registration_code)
    # print("-" * 80)
    
    # Update ServerProviderInit.h
    # print(f"\nUpdating ServerProviderInit.h...")
    result = update_server_factory_init(library_dir, registration_code, include_statements)
    
    if result['error']:
        # print(f"Error: {result['error']}")
        sys.exit(1)
    
    if result['success']:
        # print(f"✓ Successfully updated ServerProviderInit.h")
        sys.exit(0)
    else:
        # print(f"✗ Failed to update ServerProviderInit.h")
        sys.exit(1)


if __name__ == "__main__":
    main()

