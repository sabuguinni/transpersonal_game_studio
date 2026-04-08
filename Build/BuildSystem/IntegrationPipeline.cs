using System;
using System.Collections.Generic;
using System.IO;
using System.Diagnostics;
using UnrealBuildTool;

namespace TranspersonalGame.Build
{
    /// <summary>
    /// Sistema de integração contínua para o Transpersonal Game Studio
    /// Gerencia builds, testes e deployment automatizado
    /// </summary>
    public class IntegrationPipeline
    {
        public enum BuildConfiguration
        {
            Debug,
            Development,
            Test,
            Shipping
        }

        public enum TargetPlatform
        {
            Win64,
            Linux,
            Mac,
            PS5,
            XboxSeriesX,
            Android,
            iOS
        }

        private readonly string ProjectPath;
        private readonly string EnginePath;
        private readonly List<string> BuildHistory;

        public IntegrationPipeline(string projectPath, string enginePath)
        {
            ProjectPath = projectPath;
            EnginePath = enginePath;
            BuildHistory = new List<string>();
        }

        /// <summary>
        /// Executa build completo com validação de dependências
        /// </summary>
        public BuildResult ExecuteFullBuild(TargetPlatform platform, BuildConfiguration config)
        {
            var result = new BuildResult();
            result.StartTime = DateTime.Now;
            result.Platform = platform;
            result.Configuration = config;

            try
            {
                // 1. Validar dependências entre agentes
                if (!ValidateAgentDependencies())
                {
                    result.Success = false;
                    result.ErrorMessage = "Dependências entre agentes não satisfeitas";
                    return result;
                }

                // 2. Build do código C++
                if (!BuildCppCode(platform, config))
                {
                    result.Success = false;
                    result.ErrorMessage = "Falha no build do código C++";
                    return result;
                }

                // 3. Cook content
                if (!CookContent(platform))
                {
                    result.Success = false;
                    result.ErrorMessage = "Falha no cook do conteúdo";
                    return result;
                }

                // 4. Package
                if (!PackageProject(platform, config))
                {
                    result.Success = false;
                    result.ErrorMessage = "Falha no packaging";
                    return result;
                }

                // 5. Executar testes automatizados
                if (!RunAutomatedTests())
                {
                    result.Success = false;
                    result.ErrorMessage = "Falha nos testes automatizados";
                    return result;
                }

                result.Success = true;
                result.BuildPath = GetBuildOutputPath(platform, config);
                
                // Manter histórico das últimas 10 builds
                MaintainBuildHistory(result);
            }
            catch (Exception ex)
            {
                result.Success = false;
                result.ErrorMessage = ex.Message;
            }
            finally
            {
                result.EndTime = DateTime.Now;
                result.Duration = result.EndTime - result.StartTime;
            }

            return result;
        }

        /// <summary>
        /// Valida se todas as dependências entre agentes estão satisfeitas
        /// </summary>
        private bool ValidateAgentDependencies()
        {
            var dependencies = new Dictionary<string, List<string>>
            {
                ["Core"] = new List<string> { "EngineArchitect" },
                ["WorldGenerator"] = new List<string> { "Core", "Performance" },
                ["Environment"] = new List<string> { "WorldGenerator" },
                ["Architecture"] = new List<string> { "Environment" },
                ["Lighting"] = new List<string> { "Architecture" },
                ["Characters"] = new List<string> { "Lighting" },
                ["Animation"] = new List<string> { "Characters" },
                ["NPCBehavior"] = new List<string> { "Animation" },
                ["CombatAI"] = new List<string> { "NPCBehavior" },
                ["CrowdSim"] = new List<string> { "CombatAI" },
                ["Narrative"] = new List<string> { "CrowdSim" },
                ["Quests"] = new List<string> { "Narrative" },
                ["Audio"] = new List<string> { "Quests" },
                ["VFX"] = new List<string> { "Audio" },
                ["QA"] = new List<string> { "VFX" }
            };

            // Verificar se todos os outputs necessários existem
            foreach (var agent in dependencies)
            {
                foreach (var dependency in agent.Value)
                {
                    if (!CheckAgentOutput(dependency))
                    {
                        Console.WriteLine($"Dependência não satisfeita: {agent.Key} requer {dependency}");
                        return false;
                    }
                }
            }

            return true;
        }

        private bool CheckAgentOutput(string agentName)
        {
            // Verificar se o agente produziu output válido
            string agentOutputPath = Path.Combine(ProjectPath, "AgentOutputs", agentName);
            return Directory.Exists(agentOutputPath) && Directory.GetFiles(agentOutputPath).Length > 0;
        }

        private bool BuildCppCode(TargetPlatform platform, BuildConfiguration config)
        {
            string ubatPath = Path.Combine(EnginePath, "Engine", "Build", "BatchFiles", "RunUAT.bat");
            string arguments = $"BuildCookRun -project=\"{ProjectPath}\\TranspersonalGame.uproject\" " +
                             $"-platform={platform} -clientconfig={config} -build -nop4";

            return ExecuteProcess(ubatPath, arguments);
        }

        private bool CookContent(TargetPlatform platform)
        {
            string editorPath = Path.Combine(EnginePath, "Engine", "Binaries", "Win64", "UnrealEditor-Cmd.exe");
            string arguments = $"\"{ProjectPath}\\TranspersonalGame.uproject\" " +
                             $"-run=cook -targetplatform={platform} -cookonthefly=false";

            return ExecuteProcess(editorPath, arguments);
        }

        private bool PackageProject(TargetPlatform platform, BuildConfiguration config)
        {
            string ubatPath = Path.Combine(EnginePath, "Engine", "Build", "BatchFiles", "RunUAT.bat");
            string arguments = $"BuildCookRun -project=\"{ProjectPath}\\TranspersonalGame.uproject\" " +
                             $"-platform={platform} -clientconfig={config} -cook -package -stage";

            return ExecuteProcess(ubatPath, arguments);
        }

        private bool RunAutomatedTests()
        {
            // Executar testes de fumaça básicos
            // TODO: Integrar com sistema de testes do UE5
            return true;
        }

        private bool ExecuteProcess(string fileName, string arguments)
        {
            try
            {
                var process = new Process
                {
                    StartInfo = new ProcessStartInfo
                    {
                        FileName = fileName,
                        Arguments = arguments,
                        UseShellExecute = false,
                        RedirectStandardOutput = true,
                        RedirectStandardError = true,
                        CreateNoWindow = true
                    }
                };

                process.Start();
                process.WaitForExit();

                return process.ExitCode == 0;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Erro ao executar processo: {ex.Message}");
                return false;
            }
        }

        private string GetBuildOutputPath(TargetPlatform platform, BuildConfiguration config)
        {
            return Path.Combine(ProjectPath, "Binaries", platform.ToString(), config.ToString());
        }

        private void MaintainBuildHistory(BuildResult result)
        {
            string historyEntry = $"{result.StartTime:yyyy-MM-dd HH:mm:ss} - {result.Platform} - {result.Configuration} - {(result.Success ? "SUCCESS" : "FAILED")}";
            BuildHistory.Add(historyEntry);

            // Manter apenas as últimas 10 builds
            if (BuildHistory.Count > 10)
            {
                BuildHistory.RemoveAt(0);
            }

            // Salvar histórico em arquivo
            string historyPath = Path.Combine(ProjectPath, "Build", "BuildHistory.txt");
            File.WriteAllLines(historyPath, BuildHistory);
        }
    }

    public class BuildResult
    {
        public bool Success { get; set; }
        public string ErrorMessage { get; set; }
        public DateTime StartTime { get; set; }
        public DateTime EndTime { get; set; }
        public TimeSpan Duration { get; set; }
        public IntegrationPipeline.TargetPlatform Platform { get; set; }
        public IntegrationPipeline.BuildConfiguration Configuration { get; set; }
        public string BuildPath { get; set; }
    }
}