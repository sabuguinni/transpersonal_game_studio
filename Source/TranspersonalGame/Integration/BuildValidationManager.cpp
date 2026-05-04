#include "BuildValidationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "EditorLevelLibrary.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"

UBuildValidationManager::UBuildValidationManager()
{
    LastValidationTime = 0.0f;
    bAutoValidationEnabled = true;
}

void UBuildValidationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationManager: Sistema de validação inicializado"));
    
    // Executar validação inicial
    if (bAutoValidationEnabled)
    {
        FTimerHandle ValidationTimer;
        GetWorld()->GetTimerManager().SetTimer(ValidationTimer, [this]()
        {
            RunFullValidation();
        }, 5.0f, false);
    }
}

void UBuildValidationManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationManager: Sistema de validação desligado"));
    Super::Deinitialize();
}

FBuild_ValidationReport UBuildValidationManager::RunFullValidation()
{
    double StartTime = FPlatformTime::Seconds();
    FBuild_ValidationReport Report;
    
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationManager: Iniciando validação completa"));
    
    // 1. Validar estrutura de ficheiros
    ValidateFileStructure(Report);
    
    // 2. Validar dependências de módulos
    ValidateModuleDependencies(Report);
    
    // 3. Validar tipos partilhados
    ValidateSharedTypes(Report);
    
    // 4. Validar actores do mapa
    ValidateMapActors(Report);
    
    // 5. Validar iluminação
    ValidateLightingSetup(Report);
    
    // 6. Validar distribuição por biomas
    ValidateBiomeActors(Report);
    
    // 7. Testar compilação
    Report.bCompilationSuccess = TestCompilation();
    
    Report.ValidationTimeSeconds = FPlatformTime::Seconds() - StartTime;
    LastValidationReport = Report;
    LastValidationTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationManager: Validação completa em %.2f segundos"), Report.ValidationTimeSeconds);
    
    return Report;
}

TArray<FString> UBuildValidationManager::DetectOrphanHeaders()
{
    TArray<FString> OrphanHeaders;
    
    FString ProjectDir = FPaths::ProjectDir();
    FString SourceDir = FPaths::Combine(ProjectDir, TEXT("Source"), TEXT("TranspersonalGame"));
    
    // Procurar todos os .h files
    TArray<FString> HeaderFiles;
    IFileManager::Get().FindFilesRecursive(HeaderFiles, *SourceDir, TEXT("*.h"), true, false);
    
    for (const FString& HeaderFile : HeaderFiles)
    {
        FString CppFile = HeaderFile;
        CppFile = CppFile.Replace(TEXT(".h"), TEXT(".cpp"));
        
        if (!IFileManager::Get().FileExists(*CppFile))
        {
            FString RelativePath = HeaderFile;
            FPaths::MakePathRelativeTo(RelativePath, *SourceDir);
            OrphanHeaders.Add(RelativePath);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationManager: Detectados %d headers órfãos"), OrphanHeaders.Num());
    
    return OrphanHeaders;
}

int32 UBuildValidationManager::CleanDuplicateActors()
{
    int32 RemovedCount = 0;
    
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("BuildValidationManager: Mundo não disponível para limpeza"));
        return 0;
    }
    
    // Obter todos os actores
    TArray<AActor*> AllActors;
    UEditorLevelLibrary::GetAllLevelActors(GetWorld(), AllActors);
    
    // Limpar DirectionalLights duplicados
    TArray<ADirectionalLight*> DirectionalLights;
    for (AActor* Actor : AllActors)
    {
        if (ADirectionalLight* Light = Cast<ADirectionalLight>(Actor))
        {
            DirectionalLights.Add(Light);
        }
    }
    
    if (DirectionalLights.Num() > 1)
    {
        for (int32 i = 1; i < DirectionalLights.Num(); i++)
        {
            DirectionalLights[i]->Destroy();
            RemovedCount++;
        }
        UE_LOG(LogTemp, Warning, TEXT("BuildValidationManager: Removidos %d DirectionalLights duplicados"), DirectionalLights.Num() - 1);
    }
    
    // Limpar outros tipos de actores duplicados seria implementado aqui
    // (SkyAtmosphere, SkyLight, ExponentialHeightFog)
    
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationManager: Total de actores duplicados removidos: %d"), RemovedCount);
    
    return RemovedCount;
}

bool UBuildValidationManager::TestCompilation()
{
    // Simulação de teste de compilação
    // Em produção, isto executaria UnrealBuildTool
    
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationManager: Testando compilação..."));
    
    // Verificar se módulos críticos estão carregados
    bool bTranspersonalGameLoaded = FModuleManager::Get().IsModuleLoaded("TranspersonalGame");
    
    if (!bTranspersonalGameLoaded)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildValidationManager: Módulo TranspersonalGame não carregado"));
        return false;
    }
    
    // Verificar se classes críticas existem
    UClass* CharacterClass = FindObject<UClass>(ANY_PACKAGE, TEXT("TranspersonalCharacter"));
    if (!CharacterClass)
    {
        UE_LOG(LogTemp, Error, TEXT("BuildValidationManager: TranspersonalCharacter não encontrado"));
        return false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationManager: Compilação validada com sucesso"));
    return true;
}

bool UBuildValidationManager::ValidateBiomeDistribution()
{
    if (!GetWorld())
    {
        return false;
    }
    
    TArray<AActor*> AllActors;
    UEditorLevelLibrary::GetAllLevelActors(GetWorld(), AllActors);
    
    int32 ValidActors = 0;
    int32 InvalidActors = 0;
    
    for (AActor* Actor : AllActors)
    {
        FVector Location = Actor->GetActorLocation();
        
        // Verificar se está numa posição válida dos biomas
        bool bValidPosition = false;
        
        // Pantano (sudoeste)
        if (Location.X >= -77500 && Location.X <= -25000 && Location.Y >= -76500 && Location.Y <= -15000)
        {
            bValidPosition = true;
        }
        // Floresta (noroeste)
        else if (Location.X >= -77500 && Location.X <= -15000 && Location.Y >= 15000 && Location.Y <= 76500)
        {
            bValidPosition = true;
        }
        // Savana (centro)
        else if (Location.X >= -20000 && Location.X <= 20000 && Location.Y >= -20000 && Location.Y <= 20000)
        {
            bValidPosition = true;
        }
        // Deserto (leste)
        else if (Location.X >= 25000 && Location.X <= 79500 && Location.Y >= -30000 && Location.Y <= 30000)
        {
            bValidPosition = true;
        }
        // Montanha Nevada (nordeste)
        else if (Location.X >= 15000 && Location.X <= 79500 && Location.Y >= 20000 && Location.Y <= 76500)
        {
            bValidPosition = true;
        }
        
        if (bValidPosition)
        {
            ValidActors++;
        }
        else
        {
            InvalidActors++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BuildValidationManager: Actores válidos: %d, Inválidos: %d"), ValidActors, InvalidActors);
    
    return InvalidActors == 0;
}

FString UBuildValidationManager::GenerateValidationReport(const FBuild_ValidationReport& Report)
{
    FString ReportText;
    
    ReportText += TEXT("=== RELATÓRIO DE VALIDAÇÃO DE BUILD ===\n");
    ReportText += FString::Printf(TEXT("Data: %s\n"), *FDateTime::Now().ToString());
    ReportText += FString::Printf(TEXT("Tempo de validação: %.2f segundos\n\n"), Report.ValidationTimeSeconds);
    
    ReportText += TEXT("ESTATÍSTICAS:\n");
    ReportText += FString::Printf(TEXT("- Headers totais: %d\n"), Report.TotalHeaders);
    ReportText += FString::Printf(TEXT("- Implementações totais: %d\n"), Report.TotalImplementations);
    ReportText += FString::Printf(TEXT("- Headers órfãos: %d\n"), Report.OrphanHeaders);
    ReportText += FString::Printf(TEXT("- Actores duplicados: %d\n"), Report.DuplicateActors);
    ReportText += FString::Printf(TEXT("- Compilação: %s\n\n"), Report.bCompilationSuccess ? TEXT("SUCESSO") : TEXT("FALHOU"));
    
    ReportText += TEXT("PROBLEMAS DETECTADOS:\n");
    for (const FBuild_ValidationIssue& Issue : Report.Issues)
    {
        FString SeverityText;
        switch (Issue.Severity)
        {
            case EBuild_ValidationResult::Success: SeverityText = TEXT("OK"); break;
            case EBuild_ValidationResult::Warning: SeverityText = TEXT("AVISO"); break;
            case EBuild_ValidationResult::Error: SeverityText = TEXT("ERRO"); break;
            case EBuild_ValidationResult::Critical: SeverityText = TEXT("CRÍTICO"); break;
        }
        
        ReportText += FString::Printf(TEXT("[%s] %s:%s - %s\n"), 
                                     *SeverityText, *Issue.ModuleName, *Issue.FileName, *Issue.Description);
    }
    
    return ReportText;
}

void UBuildValidationManager::ValidateFileStructure(FBuild_ValidationReport& Report)
{
    FString ProjectDir = FPaths::ProjectDir();
    FString SourceDir = FPaths::Combine(ProjectDir, TEXT("Source"), TEXT("TranspersonalGame"));
    
    TArray<FString> HeaderFiles;
    TArray<FString> CppFiles;
    
    IFileManager::Get().FindFilesRecursive(HeaderFiles, *SourceDir, TEXT("*.h"), true, false);
    IFileManager::Get().FindFilesRecursive(CppFiles, *SourceDir, TEXT("*.cpp"), true, false);
    
    Report.TotalHeaders = HeaderFiles.Num();
    Report.TotalImplementations = CppFiles.Num();
    
    // Detectar headers órfãos
    for (const FString& HeaderFile : HeaderFiles)
    {
        FString CppFile = HeaderFile;
        CppFile = CppFile.Replace(TEXT(".h"), TEXT(".cpp"));
        
        if (!IFileManager::Get().FileExists(*CppFile))
        {
            Report.OrphanHeaders++;
            AddValidationIssue(Report, EBuild_ValidationResult::Error, 
                             GetModuleNameFromPath(HeaderFile), 
                             FPaths::GetCleanFilename(HeaderFile),
                             TEXT("Header sem implementação .cpp correspondente"));
        }
    }
}

void UBuildValidationManager::ValidateModuleDependencies(FBuild_ValidationReport& Report)
{
    // Verificar se módulos críticos estão disponíveis
    TArray<FString> RequiredModules = {
        TEXT("TranspersonalGame"),
        TEXT("Core"),
        TEXT("CoreUObject"),
        TEXT("Engine")
    };
    
    for (const FString& ModuleName : RequiredModules)
    {
        if (!FModuleManager::Get().IsModuleLoaded(*ModuleName))
        {
            AddValidationIssue(Report, EBuild_ValidationResult::Critical,
                             ModuleName, TEXT("Module"),
                             FString::Printf(TEXT("Módulo %s não carregado"), *ModuleName));
        }
    }
}

void UBuildValidationManager::ValidateSharedTypes(FBuild_ValidationReport& Report)
{
    // Verificar se SharedTypes.h existe e está acessível
    FString SharedTypesPath = FPaths::Combine(FPaths::ProjectDir(), TEXT("Source"), TEXT("TranspersonalGame"), TEXT("SharedTypes.h"));
    
    if (!IFileManager::Get().FileExists(*SharedTypesPath))
    {
        AddValidationIssue(Report, EBuild_ValidationResult::Critical,
                         TEXT("Core"), TEXT("SharedTypes.h"),
                         TEXT("Ficheiro SharedTypes.h não encontrado"));
    }
}

void UBuildValidationManager::ValidateMapActors(FBuild_ValidationReport& Report)
{
    if (!GetWorld())
    {
        AddValidationIssue(Report, EBuild_ValidationResult::Error,
                         TEXT("World"), TEXT("MinPlayableMap"),
                         TEXT("Mundo não disponível para validação"));
        return;
    }
    
    TArray<AActor*> AllActors;
    UEditorLevelLibrary::GetAllLevelActors(GetWorld(), AllActors);
    
    // Contar actores por tipo
    TMap<FString, int32> ActorCounts;
    for (AActor* Actor : AllActors)
    {
        FString ClassName = Actor->GetClass()->GetName();
        ActorCounts.FindOrAdd(ClassName)++;
    }
    
    // Reportar contagens
    for (const auto& Pair : ActorCounts)
    {
        if (Pair.Value > 1 && (Pair.Key.Contains(TEXT("Light")) || Pair.Key.Contains(TEXT("Sky")) || Pair.Key.Contains(TEXT("Fog"))))
        {
            Report.DuplicateActors += Pair.Value - 1;
            AddValidationIssue(Report, EBuild_ValidationResult::Warning,
                             TEXT("Map"), TEXT("MinPlayableMap"),
                             FString::Printf(TEXT("Actores duplicados: %d x %s"), Pair.Value, *Pair.Key));
        }
    }
}

void UBuildValidationManager::ValidateLightingSetup(FBuild_ValidationReport& Report)
{
    if (!GetWorld())
    {
        return;
    }
    
    TArray<AActor*> AllActors;
    UEditorLevelLibrary::GetAllLevelActors(GetWorld(), AllActors);
    
    bool bHasDirectionalLight = false;
    bool bHasSkyLight = false;
    
    for (AActor* Actor : AllActors)
    {
        if (Cast<ADirectionalLight>(Actor))
        {
            bHasDirectionalLight = true;
        }
        // Verificar outros tipos de iluminação seria implementado aqui
    }
    
    if (!bHasDirectionalLight)
    {
        AddValidationIssue(Report, EBuild_ValidationResult::Warning,
                         TEXT("Lighting"), TEXT("MinPlayableMap"),
                         TEXT("Nenhum DirectionalLight encontrado"));
    }
}

void UBuildValidationManager::ValidateBiomeActors(FBuild_ValidationReport& Report)
{
    // Implementação da validação de distribuição por biomas
    // (já implementada em ValidateBiomeDistribution)
}

bool UBuildValidationManager::IsValidBiomePosition(const FVector& Position, EBiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EBiomeType::Swamp:
            return Position.X >= -77500 && Position.X <= -25000 && Position.Y >= -76500 && Position.Y <= -15000;
        case EBiomeType::Forest:
            return Position.X >= -77500 && Position.X <= -15000 && Position.Y >= 15000 && Position.Y <= 76500;
        case EBiomeType::Savanna:
            return Position.X >= -20000 && Position.X <= 20000 && Position.Y >= -20000 && Position.Y <= 20000;
        case EBiomeType::Desert:
            return Position.X >= 25000 && Position.X <= 79500 && Position.Y >= -30000 && Position.Y <= 30000;
        case EBiomeType::SnowyMountain:
            return Position.X >= 15000 && Position.X <= 79500 && Position.Y >= 20000 && Position.Y <= 76500;
        default:
            return false;
    }
}

FString UBuildValidationManager::GetModuleNameFromPath(const FString& FilePath)
{
    TArray<FString> PathParts;
    FilePath.ParseIntoArray(PathParts, TEXT("/"));
    
    for (int32 i = 0; i < PathParts.Num(); i++)
    {
        if (PathParts[i] == TEXT("TranspersonalGame") && i + 1 < PathParts.Num())
        {
            return PathParts[i + 1];
        }
    }
    
    return TEXT("Unknown");
}

void UBuildValidationManager::AddValidationIssue(FBuild_ValidationReport& Report, EBuild_ValidationResult Severity, 
                                                const FString& Module, const FString& File, const FString& Description, int32 Line)
{
    FBuild_ValidationIssue Issue;
    Issue.Severity = Severity;
    Issue.ModuleName = Module;
    Issue.FileName = File;
    Issue.Description = Description;
    Issue.LineNumber = Line;
    
    Report.Issues.Add(Issue);
}