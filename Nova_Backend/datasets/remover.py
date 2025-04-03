import sqlite3

def delete_last_rows(db_path, table_name, num_rows=1):
    """
    Delete the last 'num_rows' rows from a table in an SQLite database.
    
    Args:
        db_path (str): Path to the SQLite database file (.db)
        table_name (str): Name of the table to modify
        num_rows (int): Number of rows to delete from the end (default: 56)
    """
    try:
        # Connect to the SQLite database
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        
        # Get the primary key column name (assuming there is one)
        cursor.execute(f"PRAGMA table_info({table_name});")
        columns = cursor.fetchall()
        pk_column = None
        for col in columns:
            if col[5] == 1:  # The pk column has non-zero value in this position
                pk_column = col[1]
                break
        
        if not pk_column:
            # If no PK found, use rowid (SQLite's internal row identifier)
            pk_column = 'rowid'
        
        # Get the last 'num_rows' row IDs
        cursor.execute(f"SELECT {pk_column} FROM {table_name} ORDER BY {pk_column} DESC LIMIT ?;", (num_rows,))
        rows_to_delete = cursor.fetchall()
        
        if not rows_to_delete:
            print(f"No rows found in table '{table_name}'")
            return
        
        # Delete the rows
        delete_query = f"DELETE FROM {table_name} WHERE {pk_column} IN ({','.join(['?']*len(rows_to_delete))});"
        cursor.execute(delete_query, [row[0] for row in rows_to_delete])
        
        # Commit changes
        conn.commit()
        print(f"Successfully deleted last {len(rows_to_delete)} rows from table '{table_name}'")
        
    except sqlite3.Error as e:
        print(f"An error occurred: {e}")
        if conn:
            conn.rollback()
    finally:
        if conn:
            conn.close()

# Example usage:
if __name__ == "__main__":
    database_path = 'D:/Nova_Project/C++/datasets/chatbot.db'  # Replace with your .db file path
    table_to_edit = "responses"       # Replace with your table name
    delete_last_rows(database_path, table_to_edit)