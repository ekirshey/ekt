class InvalidTemplate(Exception):
    def __init__(self, template_name, message="Template with name not found"):
        self.template_name = template_name
        self.message = f"{message}: {template_name}"
        super().__init__(self.message)

class TemplateVariableNotFound(Exception):
    def __init__(self, variable_name, message="Template Variable Not Found"):
        self.variable_name = variable_name
        self.message = f"{message}: {variable_name}"
        super().__init__(self.message)

class InvalidTemplateVariableType(Exception):
    def __init__(self, variable_name, message="Template Variable has invalid type (str and func only)"):
        self.variable_name = variable_name
        self.message = f"{message}: {variable_name}"
        super().__init__(self.message)

class PoorlyFormedTemplateVariable(Exception):
    def __init__(self, message="Poorly formed template"):
        self.message = f"{message}"
        super().__init__(self.message)

class PostTemplateCommandFailed(Exception):
    def __init__(self, message):
        self.message = f"{message}"
        super().__init__(self.message)