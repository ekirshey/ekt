#pragma once

#include <string>
#include <unordered_map>
#include <expected>
#include <filesystem>

#include "Context.h"

#include "ParsedTemplateString.h"
#include "Template.h"


struct EktResolvedTemplate
{
    std::string output_path;
    std::string content;
};

struct EktTemplateResult
{
    std::vector<EktResolvedTemplate> templates;
    std::vector<std::string> post_commands;
};

class Ekt
{
public:
    using InputCallback = std::function<std::string(const TemplateInputVariable& input_var)>;
    using MissingVarCallback = std::function<std::string(const std::string&)>;

    using Result = std::expected<EktTemplateResult, std::string>;
    Ekt();
    std::expected<void, std::string> initialize(const std::vector<std::filesystem::path>& scripts);
    void add_template(const std::string& name, const Template& ekt_template);
    void add_global_var(const std::string& key, const std::string& value);
    Result resolve_template(const std::string& template_name, const InputCallback& input_cb, const MissingVarCallback& missing_var_cb);
    bool template_exists(const std::string& template_name);
    std::vector<std::string> available_templates();
private:
    std::expected<void, std::string> resolve_template_impl(const std::string& template_name, const InputCallback& input_cb, const MissingVarCallback& missing_var_cb, EktTemplateResult& out);

    void get_user_input_variables(Context& context, const Template& selected_template, const InputCallback& input_cb);
    void get_missing_variables(Context& context, const Template& selected_template, const ParsedTemplateString& parsed_template, const MissingVarCallback& missing_var_cb);
    std::expected<void, std::string> resolve_functions(Context& context, const Template& selected_template);

    std::unordered_map<std::string, Template> m_templates;
    Context m_global_context;
};
