import sqlite3
import csv
import os

def sanitize_for_print(text):
    """Clean text only for console printing (keeps original for DB)"""
    return text.encode('ascii', 'replace').decode('ascii')

def insert_into_db(csv_file, db_file):
    # Check if CSV exists
    if not os.path.exists(csv_file):
        print(f"Error: CSV file {csv_file} not found.")
        return

    # Connect to DB with UTF-8 support
    conn = sqlite3.connect(db_file)
    cursor = conn.cursor()

    # Create table with correct schema (if not exists)
    cursor.execute("""
    CREATE TABLE IF NOT EXISTS responses (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        topic TEXT,
        response TEXT,
        confidence REAL
    )
    """)

    # Read and insert data
    with open(csv_file, 'r', encoding='utf-8') as file:
        csv_reader = csv.DictReader(file)
        for line_number, row in enumerate(csv_reader, 1):
            try:
                # Extract data (using your actual column names)
                topic = row['text']
                response = row['response']
                confidence = float(row['weight'])
                
                # Insert into database
                cursor.execute("""
                INSERT INTO responses (topic, response, confidence)
                VALUES (?, ?, ?)
                """, (topic, response, confidence))
                conn.commit()

                # Safe print for Windows
                print(f"Inserted line {line_number}: " + 
                      sanitize_for_print(f"topic='{topic[:30]}'...") + " " +
                      f"confidence={confidence:.2f}")
                      
            except (ValueError, KeyError) as e:
                print(f"Skipping row {line_number}: {str(e)}")
                conn.rollback()
            except sqlite3.Error as e:
                print(f"Database error on row {line_number}: {str(e)}")
                conn.rollback()

    conn.close()
    print("Data insertion complete!")

# Configure paths
csv_file = 'datasets/intents.csv'
db_file = 'D:/Nova_Project/Nova_Backend/chatbot.db'

# Run with forced UTF-8 output
import sys
if sys.stdout.encoding != 'UTF-8':
    sys.stdout.reconfigure(encoding='utf-8')
insert_into_db(csv_file, db_file)