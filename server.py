from flask import Flask, request, jsonify

app = Flask(__name__)

@app.route("/", methods=["POST"])
def receive_data():
    # Get the JSON data from the request
    data = request.get_json()
    
    # Check if data exists and contains the 'devices' key
    if data:
        print("Received JSON data:")
        print(data)  # Print the full JSON data
        if "devices" in data:
            print("Nearby Devices:", data["devices"])
    
    return jsonify({"status": "received"}), 200

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
