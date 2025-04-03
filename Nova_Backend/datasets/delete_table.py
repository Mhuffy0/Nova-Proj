import sqlite3
import os
import sys

def safe_input(prompt):
    """Unicode-safe input prompt for Windows"""
    try:
        return input(prompt)
    except UnicodeEncodeError:
        # Fallback for Windows console
        print(prompt.encode('ascii', 'replace').decode('ascii'), end='')
        return input()

def safe_print(text):
    """Unicode-safe printing for Windows"""
    try:
        print(text)
    except UnicodeEncodeError:
        print(text.encode('ascii', 'replace').decode('ascii'))

def delete_table(db_file, table_name):
    """
    Safely deletes a table from SQLite database
    
    Args:
        db_file (str): Path to SQLite database
        table_name (str): Name of table to delete
    """
    conn = None
    try:
        # Connect to database
        conn = sqlite3.connect(db_file)
        cursor = conn.cursor()
        
        # Safety check - verify table exists
        cursor.execute("SELECT name FROM sqlite_master WHERE type='table' AND name=?", (table_name,))
        if not cursor.fetchone():
            safe_print(f"[WARNING] Table '{table_name}' doesn't exist in database")
            return False
        
        # Confirm deletion with safe input
        confirm = safe_input(f"[CONFIRM] Really delete table '{table_name}'? (y/n): ").lower()
        if confirm != 'y':
            safe_print("[CANCELLED] Deletion aborted")
            return False
        
        # Execute deletion
        cursor.execute(f"DROP TABLE {table_name}")
        conn.commit()
        safe_print(f"[SUCCESS] Table '{table_name}' deleted successfully")
        return True
        
    except sqlite3.Error as e:
        safe_print(f"[ERROR] Database operation failed: {e}")
        return False
    finally:
        if conn:
            conn.close()

def list_tables(db_file):
    """List all tables in the database"""
    conn = None
    try:
        conn = sqlite3.connect(db_file)
        cursor = conn.cursor()
        cursor.execute("SELECT name FROM sqlite_master WHERE type='table' ORDER BY name")
        tables = cursor.fetchall()
        safe_print("\n=== Current Tables ===")
        safe_print("\n".join(f"- {t[0]}" for t in tables) if tables else "- No tables found")
        safe_print("")
    except sqlite3.Error as e:
        safe_print(f"[ERROR] Could not list tables: {e}")
    finally:
        if conn:
            conn.close()

if __name__ == "__main__":
    # Configure these to match your environment
    db_path = "d:/Nova_Project/C++/chatbot.db"
    table_to_delete = "word_vectors"
    
    # List tables before and after
    list_tables(db_path)
    if delete_table(db_path, table_to_delete):
        list_tables(db_path)