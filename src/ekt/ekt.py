from .exceptions import *
from .parser import parse
import subprocess
import argparse
import os

class EktTemplateComponent:
    def __init__(self, input, output):
        self.input = input
        self.output = output

class EktTemplateCommand:
    def __init__(self, command, cwd, print_stdout = False):
        self.command = command
        self.cwd = cwd
        self.print_stdout = print_stdout

class EktTemplate:
    def __init__(self):
        self.components = []
        self.user_input = []
        self.template_context = {}
        self.post_command = None
        self.chained_templates = []

class Ekt:
    def __init__(self, name):
        self.name = name
        self.context = {}
        self.templates = {}

    def run_post_command(self, post_command):
        command = post_command.command
        cwd = post_command.cwd
        print(f"Running command: {command}")
        shell = "powershell"
        command_arg = "-Command"
        if os.name != "nt":
            shell = "zsh"
            command_arg = "-c"
        result = subprocess.run(
            [shell, command_arg, command],
            cwd=cwd,
            capture_output=True,
            text=True
        )
        if result.stderr:
            raise PostTemplateCommandFailed(f"Error: Failed to run command\n Command: {command} \n Directory: {cwd} \n Return Code: {result.returncode}\n\n {result.stderr}")
        elif post_command.print_stdout:
            print(result.stdout)

    def process_template(self, template_name):
        if template_name not in self.templates:
            raise InvalidTemplate(template_name)

        active_template = self.templates[template_name]

        for user_input in active_template.user_input:
            self.context[user_input] = parse(self.context, input(f"{user_input}:  "))

        for key, value in active_template.template_context.items():
            self.context[key] = parse(self.context,value)

        for component in active_template.components:
            new_content = ""
            parsed_input_file = parse(self.context, component.input)
            with open(parsed_input_file, "r") as file:
                input_content = file.read()
                new_content = parse(self.context, input_content)

            parsed_output_file = parse(self.context, component.output)
            with open(parsed_output_file, "w") as file:
                print(f"Writing resolved content to {parsed_output_file}")
                file.write(new_content)

        for next in active_template.chained_templates:
            self.process_template(next)

        if active_template.post_command != None:
            self.run_post_command(active_template.post_command)

    def set_global_context(self, context):
        for key, item in context.items():
            self.context[key.lower()] = item

    def add_template(self, name, template):
        self.templates[name] = template

    def run(self):
        parser = argparse.ArgumentParser(description=f"Template generator for {self.name}")
        parser.add_argument("template_name", nargs="?", help="Template to run")

        args = parser.parse_args()

        if args.template_name is None:
            template_list = "\n".join(self.templates)
            print(f"Available templates:\n{template_list}")
            return

        self.process_template(args.template_name)
