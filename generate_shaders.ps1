param (
    [string]$shaders_dir,
    [string]$output_file
)

$output_content = New-Object System.Text.StringBuilder
[void]$output_content.AppendLine('#ifndef SHADERS_SOURCE_HPP')
[void]$output_content.AppendLine('#define SHADERS_SOURCE_HPP')
[void]$output_content.AppendLine('// Этот файл сгенерирован автоматически. Не редактировать вручную.')
[void]$output_content.AppendLine()

$shader_files = Get-ChildItem -Path $shaders_dir -Filter "*.glsl" -File -Recurse

foreach ($file in $shader_files) {
    $file_name = [System.IO.Path]::GetFileNameWithoutExtension($file.FullName)
    $variable_name = "shader_source_" + $file_name.Replace(".", "_")

    $file_content = Get-Content -Raw -Path $file.FullName

    $string_literal = "const char* " + $variable_name + " = R`"(" + $file_content + ")`";"
    [void]$output_content.AppendLine($string_literal)
    [void]$output_content.AppendLine()
}

[void]$output_content.AppendLine('#endif//SHADERS_SOURCE_HPP')

Set-Content -Path $output_file -Value $output_content.ToString() -Encoding UTF8
Write-Host "Успешно сгенерирован заголовочный файл $output_file"
