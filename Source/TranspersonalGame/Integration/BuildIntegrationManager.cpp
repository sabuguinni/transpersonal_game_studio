#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "Engine/GameInstance.h"
#include "TranspersonalGame.h"

DEFINE_LOG_CATEGORY(LogBuildIntegration);

UBuildIntegrationManager::UBuildIntegrationManager()
{
    bIsEnabled = true;
    LastValidationTime = FDateTime::MinValue();
    ValidationIntervalSeconds = 300.0f; // 5 minutos
    
    // Inicializar contadores
    TotalHeaderFiles = 0;
    TotalCppFiles = 0;
    OrphanHeaders = 0;
    CompilationErrors = 0;
    
    // Inicializar estado de compilação
    LastCompilationResult = EBuild_CompilationResult::Unknown;
    LastCompilationTime = FDateTime::MinValue();
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogBuildIntegration, Warning, TEXT("BuildIntegrationManager inicializado"));
    
    // Executar validação inicial
    ValidateProjectStructure();
    
    // Configurar timer para validações periódicas
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ValidationTimerHandle,
            this,
            &UBuildIntegrationManager::PeriodicValidation,
            ValidationIntervalSeconds,
            true
        );
    }
}

void UBuildIntegrationManager::Deinitialize()
{
    // Limpar timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ValidationTimerHandle);
    }
    
    Super::Deinitialize();
}

bool UBuildIntegrationManager::ValidateProjectStructure()
{
    UE_LOG(LogBuildIntegration, Warning, TEXT("Iniciando validação da estrutura do projecto"));
    
    // Reset contadores
    TotalHeaderFiles = 0;
    TotalCppFiles = 0;
    OrphanHeaders = 0;
    OrphanHeadersList.Empty();
    
    // Obter directório do projecto
    FString ProjectDir = FPaths::ProjectDir();
    FString SourceDir = FPaths::Combine(ProjectDir, TEXT("Source"), TEXT("TranspersonalGame"));
    
    // Verificar se o directório existe
    if (!IFileManager::Get().DirectoryExists(*SourceDir))
    {
        UE_LOG(LogBuildIntegration, Error, TEXT("Directório Source não encontrado: %s"), *SourceDir);
        return false;
    }
    
    // Procurar ficheiros .h e .cpp
    TArray<FString> HeaderFiles;
    TArray<FString> CppFiles;
    
    IFileManager::Get().FindFilesRecursive(HeaderFiles, *SourceDir, TEXT("*.h"), true, false);
    IFileManager::Get().FindFilesRecursive(CppFiles, *SourceDir, TEXT("*.cpp"), true, false);
    
    TotalHeaderFiles = HeaderFiles.Num();
    TotalCppFiles = CppFiles.Num();
    
    UE_LOG(LogBuildIntegration, Warning, TEXT("Encontrados %d ficheiros .h e %d ficheiros .cpp"), 
           TotalHeaderFiles, TotalCppFiles);
    
    // Verificar headers órfãos
    for (const FString& HeaderFile : HeaderFiles)
    {
        FString BaseName = FPaths::GetBaseFilename(HeaderFile);
        FString HeaderDir = FPaths::GetPath(HeaderFile);
        FString ExpectedCppFile = FPaths::Combine(HeaderDir, BaseName + TEXT(".cpp"));
        
        if (!IFileManager::Get().FileExists(*ExpectedCppFile))
        {
            OrphanHeaders++;
            OrphanHeadersList.Add(HeaderFile);
            
            UE_LOG(LogBuildIntegration, Warning, TEXT("Header órfão encontrado: %s"), *HeaderFile);
        }
    }
    
    // Actualizar timestamp
    LastValidationTime = FDateTime::Now();
    
    UE_LOG(LogBuildIntegration, Warning, TEXT("Validação concluída: %d headers órfãos de %d totais"), 
           OrphanHeaders, TotalHeaderFiles);
    
    return OrphanHeaders == 0;
}

bool UBuildIntegrationManager::TestCompilation()
{
    UE_LOG(LogBuildIntegration, Warning, TEXT("Iniciando teste de compilação"));
    
    LastCompilationTime = FDateTime::Now();
    LastCompilationResult = EBuild_CompilationResult::InProgress;
    
    // Simular teste de compilação (em produção seria chamada ao UBT)
    // Por agora, assumir sucesso se não há headers órfãos
    if (OrphanHeaders == 0)
    {
        LastCompilationResult = EBuild_CompilationResult::Success;
        CompilationErrors = 0;
        UE_LOG(LogBuildIntegration, Warning, TEXT("✅ Compilação simulada: SUCESSO"));
        return true;
    }
    else
    {
        LastCompilationResult = EBuild_CompilationResult::Failed;
        CompilationErrors = OrphanHeaders; // Simplificação
        UE_LOG(LogBuildIntegration, Error, TEXT("❌ Compilação simulada: FALHOU (%d erros)"), CompilationErrors);
        return false;
    }
}

void UBuildIntegrationManager::PeriodicValidation()
{
    if (!bIsEnabled)
    {
        return;
    }
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Executando validação periódica"));
    
    ValidateProjectStructure();
    TestCompilation();
}

FBuild_ValidationReport UBuildIntegrationManager::GetValidationReport() const
{
    FBuild_ValidationReport Report;
    
    Report.ValidationTime = LastValidationTime;
    Report.TotalHeaderFiles = TotalHeaderFiles;
    Report.TotalCppFiles = TotalCppFiles;
    Report.OrphanHeaders = OrphanHeaders;
    Report.OrphanHeadersList = OrphanHeadersList;
    Report.CompilationResult = LastCompilationResult;
    Report.CompilationErrors = CompilationErrors;
    Report.LastCompilationTime = LastCompilationTime;
    
    return Report;
}

void UBuildIntegrationManager::SetValidationEnabled(bool bEnabled)
{
    bIsEnabled = bEnabled;
    UE_LOG(LogBuildIntegration, Warning, TEXT("Validação %s"), bEnabled ? TEXT("activada") : TEXT("desactivada"));
}

void UBuildIntegrationManager::ForceValidation()
{
    UE_LOG(LogBuildIntegration, Warning, TEXT("Forçando validação manual"));
    ValidateProjectStructure();
    TestCompilation();
}

TArray<FString> UBuildIntegrationManager::GetOrphanHeaders() const
{
    return OrphanHeadersList;
}

int32 UBuildIntegrationManager::GetOrphanHeaderCount() const
{
    return OrphanHeaders;
}

EBuild_CompilationResult UBuildIntegrationManager::GetLastCompilationResult() const
{
    return LastCompilationResult;
}

int32 UBuildIntegrationManager::GetCompilationErrorCount() const
{
    return CompilationErrors;
}