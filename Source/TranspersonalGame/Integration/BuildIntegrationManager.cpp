#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "Engine/GameInstance.h"
#include "TranspersonalGame/SharedTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogBuildIntegration, Log, All);

UBuildIntegrationManager::UBuildIntegrationManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Inicializar estado de integração
    IntegrationState = EBuild_IntegrationState::Initializing;
    LastValidationTime = FDateTime::Now();
    ValidationInterval = 300.0f; // 5 minutos
    
    // Configurar validação automática
    bAutoValidationEnabled = true;
    bContinuousIntegration = true;
    MaxErrorsBeforeHalt = 10;
    
    UE_LOG(LogBuildIntegration, Log, TEXT("BuildIntegrationManager initialized"));
}

void UBuildIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Iniciar validação automática
    if (bAutoValidationEnabled)
    {
        StartIntegrationValidation();
    }
}

void UBuildIntegrationManager::StartIntegrationValidation()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Starting integration validation"));
    
    IntegrationState = EBuild_IntegrationState::Validating;
    
    // Validar estrutura de ficheiros
    ValidateFileStructure();
    
    // Validar dependências de módulos
    ValidateModuleDependencies();
    
    // Validar actores no mapa
    ValidateMapActors();
    
    // Validar sistemas críticos
    ValidateCriticalSystems();
    
    IntegrationState = EBuild_IntegrationState::Completed;
    LastValidationTime = FDateTime::Now();
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Integration validation completed"));
}

bool UBuildIntegrationManager::ValidateFileStructure()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Validating file structure"));
    
    TArray<FString> RequiredDirectories = {
        TEXT("Source/TranspersonalGame/Core"),
        TEXT("Source/TranspersonalGame/Characters"),
        TEXT("Source/TranspersonalGame/World"),
        TEXT("Source/TranspersonalGame/AI"),
        TEXT("Source/TranspersonalGame/Environment")
    };
    
    FString ProjectDir = FPaths::ProjectDir();
    bool bAllDirectoriesExist = true;
    
    for (const FString& Directory : RequiredDirectories)
    {
        FString FullPath = FPaths::Combine(ProjectDir, Directory);
        if (!IFileManager::Get().DirectoryExists(*FullPath))
        {
            UE_LOG(LogBuildIntegration, Warning, TEXT("Missing required directory: %s"), *Directory);
            bAllDirectoriesExist = false;
        }
    }
    
    // Verificar ficheiros críticos
    TArray<FString> CriticalFiles = {
        TEXT("Source/TranspersonalGame/TranspersonalGame.cpp"),
        TEXT("Source/TranspersonalGame/SharedTypes.h"),
        TEXT("Source/TranspersonalGame/TranspersonalGame.Build.cs")
    };
    
    for (const FString& File : CriticalFiles)
    {
        FString FullPath = FPaths::Combine(ProjectDir, File);
        if (!IFileManager::Get().FileExists(*FullPath))
        {
            UE_LOG(LogBuildIntegration, Error, TEXT("Missing critical file: %s"), *File);
            bAllDirectoriesExist = false;
        }
    }
    
    return bAllDirectoriesExist;
}

bool UBuildIntegrationManager::ValidateModuleDependencies()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Validating module dependencies"));
    
    // Verificar se todos os módulos necessários estão carregados
    TArray<FString> RequiredModules = {
        TEXT("Core"),
        TEXT("CoreUObject"),
        TEXT("Engine"),
        TEXT("UnrealEd"),
        TEXT("TranspersonalGame")
    };
    
    bool bAllModulesLoaded = true;
    
    for (const FString& ModuleName : RequiredModules)
    {
        if (!FModuleManager::Get().IsModuleLoaded(*ModuleName))
        {
            UE_LOG(LogBuildIntegration, Warning, TEXT("Module not loaded: %s"), *ModuleName);
            bAllModulesLoaded = false;
        }
    }
    
    return bAllModulesLoaded;
}

bool UBuildIntegrationManager::ValidateMapActors()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Validating map actors"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogBuildIntegration, Error, TEXT("No world available for validation"));
        return false;
    }
    
    // Contar actores por tipo
    TMap<FString, int32> ActorCounts;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            FString ClassName = Actor->GetClass()->GetName();
            ActorCounts.FindOrAdd(ClassName)++;
        }
    }
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Actor distribution:"));
    for (const auto& Pair : ActorCounts)
    {
        UE_LOG(LogBuildIntegration, Log, TEXT("  %s: %d"), *Pair.Key, Pair.Value);
    }
    
    // Verificar actores críticos
    bool bHasPlayerStart = ActorCounts.Contains(TEXT("PlayerStart"));
    bool bHasLighting = ActorCounts.Contains(TEXT("DirectionalLight"));
    
    if (!bHasPlayerStart)
    {
        UE_LOG(LogBuildIntegration, Error, TEXT("No PlayerStart found in map"));
    }
    
    if (!bHasLighting)
    {
        UE_LOG(LogBuildIntegration, Error, TEXT("No DirectionalLight found in map"));
    }
    
    return bHasPlayerStart && bHasLighting;
}

bool UBuildIntegrationManager::ValidateCriticalSystems()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Validating critical systems"));
    
    UGameInstance* GameInstance = GetGameInstance();
    if (!GameInstance)
    {
        UE_LOG(LogBuildIntegration, Error, TEXT("No GameInstance available"));
        return false;
    }
    
    // Verificar subsistemas críticos
    bool bAllSystemsValid = true;
    
    // Aqui podemos adicionar validações específicas para subsistemas
    // quando forem implementados pelos outros agentes
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Critical systems validation completed"));
    
    return bAllSystemsValid;
}

FBuild_IntegrationReport UBuildIntegrationManager::GenerateIntegrationReport()
{
    FBuild_IntegrationReport Report;
    
    Report.Timestamp = FDateTime::Now();
    Report.IntegrationState = IntegrationState;
    Report.bFileStructureValid = ValidateFileStructure();
    Report.bModuleDependenciesValid = ValidateModuleDependencies();
    Report.bMapActorsValid = ValidateMapActors();
    Report.bCriticalSystemsValid = ValidateCriticalSystems();
    
    // Calcular score geral
    int32 ValidSystems = 0;
    if (Report.bFileStructureValid) ValidSystems++;
    if (Report.bModuleDependenciesValid) ValidSystems++;
    if (Report.bMapActorsValid) ValidSystems++;
    if (Report.bCriticalSystemsValid) ValidSystems++;
    
    Report.OverallScore = (float)ValidSystems / 4.0f;
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Integration report generated - Score: %.2f"), Report.OverallScore);
    
    return Report;
}

void UBuildIntegrationManager::SetValidationInterval(float NewInterval)
{
    ValidationInterval = FMath::Max(30.0f, NewInterval); // Mínimo 30 segundos
    UE_LOG(LogBuildIntegration, Log, TEXT("Validation interval set to %.1f seconds"), ValidationInterval);
}

void UBuildIntegrationManager::EnableContinuousIntegration(bool bEnable)
{
    bContinuousIntegration = bEnable;
    UE_LOG(LogBuildIntegration, Log, TEXT("Continuous integration %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

bool UBuildIntegrationManager::IsIntegrationHealthy() const
{
    FBuild_IntegrationReport Report = const_cast<UBuildIntegrationManager*>(this)->GenerateIntegrationReport();
    return Report.OverallScore >= 0.75f; // 75% ou mais é considerado saudável
}