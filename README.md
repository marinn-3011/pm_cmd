# pm — Project Manager CLI

A lightweight command-line tool for creating, browsing, and managing local programming project folders, written in C.

`pm` maintains a projects directory where each project lives in its own folder, pre-populated with a log file and an empty main source file matching the language you specify.

Built primarily for Ubuntu. The tool will work on most Linux distros with minimal changes to source.

## Features

- **Create** new project folders with a log file and a language-specific
  main file (`.c`, `.py`, `.java`, `.cpp`)
- **View** the contents of a project (recursively), with color-coded
  directories
- **List** all existing projects at a glance
- **Delete** a project (with a confirmation prompt) recursively
- **Open** a project in your file manager, or in a terminal at that
  working directory


## Building

```sh
gcc -o pm manager.c
```

Optionally install it somewhere on your `$PATH`:

```sh
sudo cp pm /usr/local/bin/pm
```

## Configuration

Before building, edit the two variables at the top of the source file to
match your setup:

```c
char proj_path[]    = "/path/to/your/projects/folder/";
char folder_prefix[] = "Project_";
```

- `proj_path` — the parent directory where all projects are stored. Make
  sure this directory already exists and ends with a trailing slash.
- `folder_prefix` — a prefix prepended to every project's folder name.

## Usage

```sh
pm <command> <proj_name> <options>
```

### Commands

| Command | Requires `proj_name` | Description |
|---|---|---|
| `new`   | Yes | Create a new project folder |
| `view`  | Yes | List a project's contents |
| `kill`  | Yes | Permanently delete a project (asks for confirmation) |
| `list`  | No  | List all existing projects |
| `enter` | Yes | Open a project in your file manager or a terminal |

### Options

| Option | Applies to | Description |
|---|---|---|
| `-l <lang>` | `new` | Sets the main file's extension |
| `-t` | `enter` | Open the project directory in a terminal instead of the file manager |
| `-r` | `view` | View the project's contents recursively |

**Note:** Options can be placed before or after positional arguments.

### Examples

Create a new C project called `calculator`:

```sh
pm new calculator -l c
```

This creates `Project_calculator/` containing `Logfile.txt` and `main.c`.

Create a new project with no specific language:

```sh
pm new scratchpad
```

This creates an empty `main` file (no extension).

View a project's contents:

```sh
pm view calculator [-r]
```

List all projects:

```sh
pm list
```

Open a project in your file manager:

```sh
pm enter calculator
```

Open a project in a terminal at that directory:

```sh
pm enter calculator -t
```

Delete a project:

```
pm kill calculator
```

You'll be asked to confirm with `y`/`n` before anything is deleted.


## Notes & limitations

- The projects directory path and folder prefix are currently hardcoded
  at compile time — there's no runtime config file or environment
  variable override yet.
- `enter` currently launches `xdg-open` or `gnome-terminal`; if you use a
  different terminal emulator, update the corresponding call in the
  source.
- Deletion via `kill` is recursive and permanent — there is no trash or
  undo.


