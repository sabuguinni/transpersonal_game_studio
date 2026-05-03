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
    IntegrationState = EBuildIntegrationState::Idle;
    LastValidationTime = FDateTime::Now();
    ValidationInterval = 300.0f; // 5 minutos
    
    // Configurar validações críticas
    CriticalValidations.Add(TEXT("ModuleCompilation"));
    CriticalValidations.Add(TEXT("ActorSpawning"));
    CriticalValidations.Add(TEXT("ComponentRegistration"));
    CriticalValidations.Add(TEXT("AssetLoading"));
    
    UE_LOG(LogBuildIntegration, Log, TEXT("BuildIntegrationManager inicializado"));
}

void UBuildIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Executar validação inicial
    ExecuteIntegrationValidation();
}

void UBuildIntegrationManager::ExecuteIntegrationValidation()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Iniciando validação de integração crítica"));
    
    IntegrationState = EBuildIntegrationState::Validating;
    ValidationResults.Empty();
    
    // Validar compilação de módulos
    ValidateModuleCompilation();
    
    // Validar spawning de actores
    ValidateActorSpawning();
    
    // Validar registo de componentes
    ValidateComponentRegistration();
    
    // Validar carregamento de assets
    ValidateAssetLoading();
    
    // Processar resultados
    ProcessValidationResults();
    
    LastValidationTime = FDateTime::Now();
    IntegrationState = EBuildIntegrationState::Idle;
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Validação de integração concluída"));
}

void UBuildIntegrationManager::ValidateModuleCompilation()
{
    FBuildValidationResult Result;
    Result.ValidationName = TEXT("ModuleCompilation");
    Result.bPassed = true;
    Result.ErrorMessages.Empty();
    
    // Verificar se o módulo TranspersonalGame está carregado
    if (!FModuleManager::Get().IsModuleLoaded(TEXT("TranspersonalGame")))
    {
        Result.bPassed = false;
        Result.ErrorMessages.Add(TEXT("Módulo TranspersonalGame não está carregado"));
    }
    
    // Verificar classes críticas
    TArray<FString> CriticalClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameMode"),
        TEXT("/Script/TranspersonalGame.BuildIntegrationManager")
    };
    
    for (const FString& ClassName : CriticalClasses)
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
        if (!LoadedClass)
        {
            Result.bPassed = false;
            Result.ErrorMessages.Add(FString::Printf(TEXT("Classe crítica não encontrada: %s"), *ClassName));
        }
    }
    
    ValidationResults.Add(Result);
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Validação de compilação: %s"), 
           Result.bPassed ? TEXT("PASSOU") : TEXT("FALHOU"));
}

void UBuildIntegrationManager::ValidateActorSpawning()
{
    FBuildValidationResult Result;
    Result.ValidationName = TEXT("ActorSpawning");
    Result.bPassed = true;
    Result.ErrorMessages.Empty();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        Result.bPassed = false;
        Result.ErrorMessages.Add(TEXT("World não disponível para teste de spawning"));
        ValidationResults.Add(Result);
        return;
    }
    
    // Testar spawning de actor básico
    FVector TestLocation(0.0f, 0.0f, 1000.0f);
    FRotator TestRotation = FRotator::ZeroRotator;
    
    AActor* TestActor = World->SpawnActor<AActor>(AActor::StaticClass(), TestLocation, TestRotation);
    if (TestActor)
    {
        // Limpar actor de teste
        TestActor->Destroy();
        UE_LOG(LogBuildIntegration, Log, TEXT("Teste de spawning básico: SUCESSO"));
    }
    else
    {
        Result.bPassed = false;
        Result.ErrorMessages.Add(TEXT("Falha ao spawnar actor de teste"));
    }
    
    ValidationResults.Add(Result);
}

void UBuildIntegrationManager::ValidateComponentRegistration()
{
    FBuildValidationResult Result;
    Result.ValidationName = TEXT("ComponentRegistration");
    Result.bPassed = true;
    Result.ErrorMessages.Empty();
    
    // Verificar se este componente está registado correctamente
    AActor* Owner = GetOwner();
    if (Owner)
    {
        UBuildIntegrationManager* FoundComponent = Owner->FindComponentByClass<UBuildIntegrationManager>();
        if (!FoundComponent)
        {
            Result.bPassed = false;
            Result.ErrorMessages.Add(TEXT("BuildIntegrationManager não encontrado no owner"));
        }
    }
    else
    {
        Result.bPassed = false;
        Result.ErrorMessages.Add(TEXT("Owner não definido para BuildIntegrationManager"));
    }
    
    ValidationResults.Add(Result);
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Validação de componentes: %s"), 
           Result.bPassed ? TEXT("PASSOU") : TEXT("FALHOU"));
}

void UBuildIntegrationManager::ValidateAssetLoading()
{
    FBuildValidationResult Result;
    Result.ValidationName = TEXT("AssetLoading");
    Result.bPassed = true;
    Result.ErrorMessages.Empty();
    
    // Testar carregamento de assets básicos do Engine
    TArray<FString> TestAssets = {
        TEXT("/Engine/BasicShapes/Cube"),
        TEXT("/Engine/BasicShapes/Sphere"),
        TEXT("/Engine/EngineMaterials/DefaultMaterial")
    };
    
    for (const FString& AssetPath : TestAssets)
    {
        UObject* LoadedAsset = LoadObject<UObject>(nullptr, *AssetPath);
        if (!LoadedAsset)
        {
            Result.bPassed = false;
            Result.ErrorMessages.Add(FString::Printf(TEXT("Falha ao carregar asset: %s"), *AssetPath));
        }
    }
    
    ValidationResults.Add(Result);
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Validação de assets: %s"), 
           Result.bPassed ? TEXT("PASSOU") : TEXT("FALHOU"));
}

void UBuildIntegrationManager::ProcessValidationResults()
{
    int32 PassedValidations = 0;
    int32 TotalValidations = ValidationResults.Num();
    
    for (const FBuildValidationResult& Result : ValidationResults)
    {
        if (Result.bPassed)
        {
            PassedValidations++;
        }
        else
        {
            UE_LOG(LogBuildIntegration, Error, TEXT("Validação FALHOU: %s"), *Result.ValidationName);
            for (const FString& Error : Result.ErrorMessages)
            {
                UE_LOG(LogBuildIntegration, Error, TEXT("  - %s"), *Error);
            }
        }
    }
    
    float SuccessRate = TotalValidations > 0 ? (float)PassedValidations / TotalValidations * 100.0f : 0.0f;
    
    UE_LOG(LogBuildIntegration, Log, TEXT("Resultados de Integração: %d/%d validações passaram (%.1f%%)"), 
           PassedValidations, TotalValidations, SuccessRate);
    
    // Determinar estado final
    if (PassedValidations == TotalValidations)
    {
        IntegrationState = EBuildIntegrationState::Healthy;
        UE_LOG(LogBuildIntegration, Log, TEXT("✅ INTEGRAÇÃO SAUDÁVEL - Todas as validações passaram"));
    }
    else if (PassedValidations >= TotalValidations * 0.8f) // 80% ou mais
    {
        IntegrationState = EBuildIntegrationState::Warning;
        UE_LOG(LogBuildIntegration, Warning, TEXT("⚠️ INTEGRAÇÃO COM AVISOS - Algumas validações falharam"));
    }
    else
    {
        IntegrationState = EBuildIntegrationState::Critical;
        UE_LOG(LogBuildIntegration, Error, TEXT("❌ INTEGRAÇÃO CRÍTICA - Muitas validações falharam"));
    }
}

bool UBuildIntegrationManager::IsIntegrationHealthy() const
{
    return IntegrationState == EBuildIntegrationState::Healthy;
}

FString UBuildIntegrationManager::GetIntegrationReport() const
{
    FString Report = TEXT("=== RELATÓRIO DE INTEGRAÇÃO ===\n");
    
    Report += FString::Printf(TEXT("Estado: %s\n"), 
                             IntegrationState == EBuildIntegrationState::Healthy ? TEXT("SAUDÁVEL") :
                             IntegrationState == EBuildIntegrationState::Warning ? TEXT("AVISO") :
                             IntegrationState == EBuildIntegrationState::Critical ? TEXT("CRÍTICO") : TEXT("DESCONHECIDO"));
    
    Report += FString::Printf(TEXT("Última Validação: %s\n"), *LastValidationTime.ToString());
    Report += FString::Printf(TEXT("Total de Validações: %d\n"), ValidationResults.Num());
    
    int32 PassedCount = 0;
    for (const FBuildValidationResult& Result : ValidationResults)
    {
        if (Result.bPassed) PassedCount++;
    }
    
    Report += FString::Printf(TEXT("Validações Passaram: %d/%d\n"), PassedCount, ValidationResults.Num());
    
    Report += TEXT("\nDetalhes das Validações:\n");
    for (const FBuildValidationResult& Result : ValidationResults)
    {
        Report += FString::Printf(TEXT("- %s: %s\n"), 
                                 *Result.ValidationName, 
                                 Result.bPassed ? TEXT("PASSOU") : TEXT("FALHOU"));
        
        if (!Result.bPassed)
        {
            for (const FString& Error : Result.ErrorMessages)
            {
                Report += FString::Printf(TEXT("  * %s\n"), *Error);
            }
        }
    }
    
    return Report;
}

void UBuildIntegrationManager::ForceValidation()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Validação forçada solicitada"));
    ExecuteIntegrationValidation();
}