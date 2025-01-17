from flask import Flask, render_template, request, jsonify
import requests
import json
import datetime
import plotly.graph_objs as go
from plotly.utils import PlotlyJSONEncoder

app = Flask(__name__)

API_BASE_URL = "http://localhost:8080"

def fetch_temperature_data(start, end):
    try:
        response = requests.get(f"{API_BASE_URL}/stats", params={"start": start, "end": end})
        response.raise_for_status()
        return response.json()
    except requests.exceptions.RequestException as e:
        print(f"Error fetching data: {e}")
        return []

@app.route("/")
def index():
    current_time = int(datetime.datetime.now().timestamp())
    start_time = current_time - 600  # Последний час
    data = fetch_temperature_data(start_time, current_time)

    if not data:
        return render_template("index.html", table_data=[], graph_json=json.dumps({}))

    # Подготовка данных
    table_data = [
        {"timestamp": datetime.datetime.fromtimestamp(item["timestamp"]).strftime("%Y-%m-%d %H:%M:%S"), "temperature": item["temperature"]}
        for item in reversed(data)
    ]
    timestamps = [datetime.datetime.fromtimestamp(item["timestamp"]) for item in data]
    temperatures = [item["temperature"] for item in data]

    graph = {
        "data": [go.Scatter(x=timestamps, y=temperatures, mode="lines+markers", name="Temperature")],
        "layout": go.Layout(title="Temperature over Time", xaxis=dict(title="Time"), yaxis=dict(title="Temperature (°C)"), template="plotly_white"),
    }
    graph_json = json.dumps(graph, cls=PlotlyJSONEncoder)
    return render_template("index.html", table_data=table_data, graph_json=graph_json)

if __name__ == "__main__":
    app.run(debug=True)
