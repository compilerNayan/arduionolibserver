"""
Script to check if a file contains @ServerImpl annotation above a class that inherits from IServer.

The pattern should be:
    /* @ServerImpl("something_in_quotes") */
    class SomeClass : public IServer

or

    /* @ServerImpl("something_in_quotes") */
    class SomeClass final : public IServer
"""

import sys
import re
from pathlib import Path


def check_server_impl(file_path):
    """
    Check if a file contains @ServerImpl annotation above a class that inherits from IServer.
    
    Args:
        file_path: Path to the file to check (can be string or Path object)
    
    Returns:
        dict: Dictionary with 'found' (bool), 'matches' (list of match info), and 'error' (str if any)
    """
    result = {
        'found': False,
        'matches': [],
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
        
        # Resolve to full absolute path
        full_file_path = file_path.resolve()
        
        # Read the file content
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            lines = f.readlines()
        
        # Pattern to match @ServerImpl annotation followed by class declaration
        # Pattern: /* @ServerImpl("content") */ or /*@ServerImpl("content")*/ (ignoring whitespace)
        # Also check for already processed /*--@ServerImpl("content")--*/ pattern
        # Class can have optional 'final' keyword
        # Class must inherit from IServer
        server_impl_annotation_pattern = re.compile(r'/\*\s*@ServerImpl\s*\(([^)]*)\)\s*\*/')
        server_impl_processed_pattern = re.compile(r'/\*--\s*@ServerImpl\s*\(([^)]*)\)\s*--\*/')
        class_pattern = re.compile(r'class\s+(\w+)(?:\s+final)?\s*:\s*public\s+IServer')
        
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
        
        # Check each line and the next line
        for i, line in enumerate(lines):
            stripped_line = line.strip()
            
            # Skip already processed annotations
            if server_impl_processed_pattern.search(stripped_line):
                continue
            
            # Check if current line has @ServerImpl annotation
            server_impl_match = server_impl_annotation_pattern.search(stripped_line)
            if server_impl_match:
                # Extract the content inside brackets
                bracket_content = server_impl_match.group(1)
                extracted_content = extract_bracket_content(bracket_content)
                
                # Check next few lines for class declaration (allow some whitespace/comments)
                # Look ahead up to 5 lines (to handle comments or blank lines)
                for j in range(i + 1, min(i + 6, len(lines))):
                    class_match = class_pattern.search(lines[j])
                    if class_match:
                        # Found a match!
                        class_name = class_match.group(1)
                        server_impl_full = server_impl_match.group(0)
                        
                        match_info = {
                            'line_number': i + 1,  # 1-indexed
                            'server_impl_line': line.strip(),
                            'class_line': lines[j].strip(),
                            'class_name': class_name,
                            'server_impl_annotation': server_impl_full,
                            'server_impl_content': extracted_content,
                            'file_path': str(full_file_path)  # Full absolute path
                        }
                        result['matches'].append(match_info)
                        result['found'] = True
                        break  # Found match for this @ServerImpl, move to next
    
    except Exception as e:
        result['error'] = f"Error processing file: {str(e)}"
    
    return result


def main():
    """Main function to run the script from command line."""
    if len(sys.argv) < 2:
        # print("Usage: python check_server_impl.py <file_path>")
        # print("Example: python check_server_impl.py /path/to/file.h")
        sys.exit(1)
    
    file_path = sys.argv[1]
    result = check_server_impl(file_path)
    
    if result['error']:
        # print(f"Error: {result['error']}")
        sys.exit(1)
    
    if result['found']:
        # print(f"✓ Found {len(result['matches'])} @ServerImpl annotation(s) in {file_path}")
        for i, match in enumerate(result['matches'], 1):
            # print(f"\nMatch {i}:")
            # print(f"  File path: {match['file_path']}")
            # print(f"  Line {match['line_number']}: {match['server_impl_line']}")
            # print(f"  @ServerImpl content: {match['server_impl_content']}")
            # print(f"  Class: {match['class_name']}")
            # print(f"  Class line: {match['class_line']}")
            pass
        sys.exit(0)
    else:
        # print(f"✗ No @ServerImpl annotation found above IServer class in {file_path}")
        sys.exit(1)


if __name__ == "__main__":
    main()

