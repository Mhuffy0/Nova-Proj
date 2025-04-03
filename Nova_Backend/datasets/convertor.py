import json
import csv

# Load the JSON data
with open('datasets/intents.json', 'r') as f:
    data = json.load(f)

# Open a new CSV file to write the data
with open('intents.csv', 'w', newline='') as csvfile:
    fieldnames = ['intent', 'text', 'response', 'weight']  # Define the column headers
    writer = csv.DictWriter(csvfile, fieldnames=fieldnames)

    # Write the header row
    writer.writeheader()

    # Loop through each intent in the JSON data and write it to the CSV
    for intent in data['intents']:
        for pattern, response in zip(intent['patterns'], intent['responses']):
            # Assign a random weight (you can modify this based on your criteria)
            weight = 0.9  # Default weight for now
            # Write each row to the CSV file
            writer.writerow({
                'intent': intent['tag'],
                'text': pattern,
                'response': response,
                'weight': weight
            })

print("CSV conversion completed successfully.")
