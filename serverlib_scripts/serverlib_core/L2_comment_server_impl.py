"""
Script to mark @ServerImpl annotation as processed in a given file.

The script finds @ServerImpl annotations above classes that inherit from IServer
and marks them as processed by replacing /* @ServerImpl(...) */ with /*--@ServerImpl(...)--*/.
"""

import sys
import re
from pathlib import Path


def comment_server_impl(file_path, dry_run=False):
    """
    Mark @ServerImpl annotation as processed in a file.
    
    Args:
        file_path: Path to the file to modify (can be string or Path object)
        dry_run: If True, only report what would be changed without modifying the file
    
    Returns:
        dict: Dictionary with 'modified' (bool), 'commented_lines' (list), and 'error' (str if any)
    """
    result = {
        'modified': False,
        'commented_lines': [],
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
        lines_to_comment = []
        
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
                for j in range(i + 1, min(i + 6, len(lines))):
                    class_match = class_pattern.search(lines[j])
                    if class_match:
                        found_class = True
                        break
                
                # Only process if it's followed by a matching class
                if found_class:
                    # Check if already processed
                    if not server_impl_processed_pattern.search(stripped_line):
                        # Preserve indentation and replace with processed marker
                        indent = server_impl_match.group(1)
                        content_part = server_impl_match.group(2)  # Content inside parentheses
                        processed_line = f"{indent}/*--@ServerImpl({content_part})--*/\n"
                            modified_lines[i] = processed_line
                            lines_to_comment.append({
                                'line_number': i + 1,
                                'original': line.rstrip('\n'),
                                'commented': processed_line.rstrip('\n')
                            })
                            result['modified'] = True
        
        # Write the modified content if not dry run
        if result['modified'] and not dry_run:
            with open(file_path, 'w', encoding='utf-8', errors='ignore') as f:
                f.writelines(modified_lines)
        
        result['commented_lines'] = lines_to_comment
        
    except Exception as e:
        result['error'] = f"Error processing file: {str(e)}"
    
    return result


def main():
    """Main function to run the script from command line."""
    if len(sys.argv) < 2:
        # print("Usage: python L2_comment_server_impl.py <file_path> [--dry-run]")
        # print("Example: python L2_comment_server_impl.py /path/to/file.h")
        # print("         python L2_comment_server_impl.py /path/to/file.h --dry-run")
        sys.exit(1)
    
    file_path = sys.argv[1]
    dry_run = '--dry-run' in sys.argv or '-n' in sys.argv
    
    if dry_run:
        # print("DRY RUN MODE - No changes will be made to the file\n")
        pass
    
    result = comment_server_impl(file_path, dry_run=dry_run)
    
    if result['error']:
        # print(f"Error: {result['error']}")
        sys.exit(1)
    
    if result['modified']:
        action = "Would mark" if dry_run else "Marked"
        # print(f"✓ {action} {len(result['commented_lines'])} @ServerImpl annotation(s) as processed in {file_path}")
        for item in result['commented_lines']:
            # print(f"\n  Line {item['line_number']}:")
            # print(f"    Original: {item['original']}")
            # print(f"    {'Would be' if dry_run else 'Now'}: {item['commented']}")
            pass
        if not dry_run:
            # print(f"\n✓ File saved successfully")
            pass
        sys.exit(0)
    else:
        # print(f"✗ No @ServerImpl annotations found to process in {file_path}")
        sys.exit(0)


if __name__ == "__main__":
    main()

