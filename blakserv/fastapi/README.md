# Meridian 59 API

This is a FastAPI-based API for managing the **BlakSton Meridian 59** server.

## Features
- Get a list of online players
- Future: Query server runtime
- Future: Execute admin commands via API

## Installation

### 1 Install dependencies
`pip install -r requirements.txt`
(Or use Poetry, see below)

### 2 Run the server:
`uvicorn main:app --reload`

### 3 Open API documentation:

- Swagger UI: http://127.0.0.1:8000/docs
- ReDoc UI: http://127.0.0.1:8000/redoc

## Endpoints:

| Method | Endpoint           | Description         |
|--------|-------------------|---------------------|
| GET    | `/players/online` | Get online players |
| GET    | `/server/runtime` | Get server uptime  |

## Dependency Management with Poetry (Optional)

Instead of `requirements.txt`, you can use Poetry for better dependency management.

### 1 Install Poetry:
`pip install poetry`

### 2 Initialize Poetry in Your FastAPI Project:
`poetry init`
(Default answers are fine)

### 3 Add Dependencies:
`poetry add fastapi uvicorn requests`

This creates a pyproject.toml file, replacing requirements.txt.

### 4 Run Your API with Poetry:
`poetry run uvicorn main:app --reload`

## License
Meridian 59 is open-source. See `LICENSE` for details.