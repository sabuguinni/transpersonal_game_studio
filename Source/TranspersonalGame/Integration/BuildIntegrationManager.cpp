#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"
#include "Engine/GameInstance.h"

DEFINE_LOG_CATEGORY(LogBuildIntegration);

UBuildIntegrationManager::UBuildIntegrationManager()
{
    bAutoValidateOnStartup = true;
    bEnableCompilationChecks = true;
    bEnableModuleValidation = true;
    ValidationIntervalSeconds = 30.0f;
    MaxValidationErrors = 50;
    
    // Inicializar contadores
    TotalModulesFound = 0;
    ValidModulesCount = 0;
    OrphanHeadersCount = 0;
    CompilationErrorsCount = 0;
    LastValidationTime = 0.0f;
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogBuildIntegration, Warning, TEXT("BuildIntegrationManager initialized"));
    
    if (bAutoValidateOnStartup)
    {
        // Agendar validação inicial após 5 segundos
        GetWorld()->GetTimerManager().SetTimer(
            ValidationTimerHandle,
            this,
            &UBuildIntegrationManager::PerformFullValidation,
            5.0f,
            false
        );
    }
}

void UBuildIntegrationManager::Deinitialize()
{
    if (GetWorld() && ValidationTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(ValidationTimerHandle);
    }
    
    Super::Deinitialize();
}

void UBuildIntegrationManager::PerformFullValidation()
{
    UE_LOG(LogBuildIntegration, Warning, TEXT("Starting full build validation..."));
    
    LastValidationTime = GetWorld()->GetTimeSeconds();
    
    // Reset contadores
    TotalModulesFound = 0;
    ValidModulesCount = 0;
    OrphanHeadersCount = 0;
    CompilationErrorsCount = 0;
    ValidationErrors.Empty();
    
    // Validar estrutura de módulos
    ValidateModuleStructure();
    
    // Validar headers órfãos
    ValidateOrphanHeaders();
    
    // Validar dependências
    ValidateModuleDependencies();
    
    // Agendar próxima validação
    if (bEnableCompilationChecks && ValidationIntervalSeconds > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            ValidationTimerHandle,
            this,
            &UBuildIntegrationManager::PerformFullValidation,
            ValidationIntervalSeconds,
            false
        );
    }
    
    // Log resultado
    UE_LOG(LogBuildIntegration, Warning, TEXT("Validation complete: %d modules, %d valid, %d orphan headers, %d errors"),
        TotalModulesFound, ValidModulesCount, OrphanHeadersCount, ValidationErrors.Num());
}

void UBuildIntegrationManager::ValidateModuleStructure()
{
    FString ProjectDir = FPaths::ProjectDir();
    FString SourceDir = FPaths::Combine(ProjectDir, TEXT("Source"), TEXT("TranspersonalGame"));
    
    TArray<FString> HeaderFiles;
    TArray<FString> CppFiles;
    
    // Encontrar todos os ficheiros .h e .cpp
    IFileManager& FileManager = IFileManager::Get();
    FileManager.FindFilesRecursive(HeaderFiles, *SourceDir, TEXT("*.h"), true, false);
    FileManager.FindFilesRecursive(CppFiles, *SourceDir, TEXT("*.cpp"), true, false);
    
    TotalModulesFound = HeaderFiles.Num();
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Found %d header files and %d cpp files"), 
        HeaderFiles.Num(), CppFiles.Num());
    
    // Verificar pares .h/.cpp
    for (const FString& HeaderFile : HeaderFiles)
    {
        FString BaseName = FPaths::GetBaseFilename(HeaderFile);
        FString ExpectedCppFile = FPaths::Combine(FPaths::GetPath(HeaderFile), BaseName + TEXT(".cpp"));
        
        if (FileManager.FileExists(*ExpectedCppFile))
        {
            ValidModulesCount++;
        }
        else
        {
            OrphanHeadersCount++;
            AddValidationError(FString::Printf(TEXT("Orphan header: %s (missing %s)"), 
                *FPaths::GetCleanFilename(HeaderFile), 
                *FPaths::GetCleanFilename(ExpectedCppFile)));
        }
    }
}

void UBuildIntegrationManager::ValidateOrphanHeaders()
{
    // Esta validação já foi feita em ValidateModuleStructure
    // Aqui podemos adicionar validações mais específicas se necessário
    
    if (OrphanHeadersCount > 0)
    {
        UE_LOG(LogBuildIntegration, Warning, TEXT("Found %d orphan headers - these will cause compilation issues"), 
            OrphanHeadersCount);
    }
}

void UBuildIntegrationManager::ValidateModuleDependencies()
{
    // Validar dependências básicas do módulo
    FString BuildCsPath = FPaths::Combine(FPaths::ProjectDir(), TEXT("Source"), TEXT("TranspersonalGame"), TEXT("TranspersonalGame.Build.cs"));
    
    if (!IFileManager::Get().FileExists(*BuildCsPath))
    {
        AddValidationError(TEXT("Missing TranspersonalGame.Build.cs file"));
        return;
    }
    
    FString BuildCsContent;
    if (FFileHelper::LoadFileToString(BuildCsContent, *BuildCsPath))
    {
        // Verificar dependências essenciais
        TArray<FString> RequiredDependencies = {
            TEXT("Core"),
            TEXT("CoreUObject"),
            TEXT("Engine"),
            TEXT("UnrealEd")
        };
        
        for (const FString& Dependency : RequiredDependencies)
        {
            if (!BuildCsContent.Contains(Dependency))
            {
                AddValidationError(FString::Printf(TEXT("Missing required dependency: %s"), *Dependency));
            }
        }
    }
    else
    {
        AddValidationError(TEXT("Failed to read TranspersonalGame.Build.cs"));
    }
}

void UBuildIntegrationManager::AddValidationError(const FString& ErrorMessage)
{
    if (ValidationErrors.Num() < MaxValidationErrors)
    {
        ValidationErrors.Add(ErrorMessage);
        CompilationErrorsCount++;
        
        UE_LOG(LogBuildIntegration, Error, TEXT("Validation Error: %s"), *ErrorMessage);
    }
}

bool UBuildIntegrationManager::IsValidationPassing() const
{
    return ValidationErrors.Num() == 0 && OrphanHeadersCount == 0;
}

FString UBuildIntegrationManager::GetValidationSummary() const
{
    return FString::Printf(TEXT("Modules: %d/%d valid, Orphan Headers: %d, Errors: %d"),
        ValidModulesCount, TotalModulesFound, OrphanHeadersCount, ValidationErrors.Num());
}

TArray<FString> UBuildIntegrationManager::GetValidationErrors() const
{
    return ValidationErrors;
}

void UBuildIntegrationManager::ForceValidation()
{
    PerformFullValidation();
}

void UBuildIntegrationManager::SetValidationEnabled(bool bEnabled)
{
    bEnableCompilationChecks = bEnabled;
    
    if (!bEnabled && ValidationTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(ValidationTimerHandle);
    }
    else if (bEnabled)
    {
        PerformFullValidation();
    }
}