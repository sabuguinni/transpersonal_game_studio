#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "Engine/GameInstance.h"
#include "TranspersonalGame/SharedTypes.h"

DEFINE_LOG_CATEGORY(LogBuildIntegration);

UBuildIntegrationManager::UBuildIntegrationManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Inicializar estado de build
    CurrentBuildStatus = EBuildStatus::Unknown;
    LastBuildTime = FDateTime::Now();
    BuildVersion = TEXT("1.0.0");
    
    // Configurar validação automática
    bAutoValidateOnStartup = true;
    bLogDetailedErrors = true;
    ValidationInterval = 60.0f; // 1 minuto
}

void UBuildIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogBuildIntegration, Warning, TEXT("BuildIntegrationManager: Starting integration validation"));
    
    if (bAutoValidateOnStartup)
    {
        // Executar validação inicial após 2 segundos
        GetWorld()->GetTimerManager().SetTimer(
            ValidationTimerHandle,
            this,
            &UBuildIntegrationManager::PerformFullValidation,
            2.0f,
            false
        );
    }
}

void UBuildIntegrationManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Limpar timers
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ValidationTimerHandle);
    }
    
    Super::EndPlay(EndPlayReason);
}

void UBuildIntegrationManager::PerformFullValidation()
{
    UE_LOG(LogBuildIntegration, Warning, TEXT("BuildIntegrationManager: Starting full validation"));
    
    ValidationResults.Empty();
    CurrentBuildStatus = EBuildStatus::Validating;
    
    // 1. Validar classes críticas
    ValidateCriticalClasses();
    
    // 2. Validar sistemas de jogo
    ValidateGameSystems();
    
    // 3. Validar assets críticos
    ValidateCriticalAssets();
    
    // 4. Validar configuração de módulos
    ValidateModuleConfiguration();
    
    // 5. Determinar status final
    DetermineFinalBuildStatus();
    
    // 6. Log resultados
    LogValidationResults();
    
    // 7. Programar próxima validação
    if (ValidationInterval > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            ValidationTimerHandle,
            this,
            &UBuildIntegrationManager::PerformFullValidation,
            ValidationInterval,
            false
        );
    }
}

void UBuildIntegrationManager::ValidateCriticalClasses()
{
    TArray<FString> CriticalClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalGameMode"),
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.PCGWorldGenerator"),
        TEXT("/Script/TranspersonalGame.FoliageManager")
    };
    
    for (const FString& ClassName : CriticalClasses)
    {
        FBuildValidationResult Result;
        Result.TestName = FString::Printf(TEXT("Class Load: %s"), *ClassName);
        Result.Category = EValidationCategory::ClassLoading;
        
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
        if (LoadedClass)
        {
            Result.bPassed = true;
            Result.Message = TEXT("Class loaded successfully");
            UE_LOG(LogBuildIntegration, Log, TEXT("✓ %s loaded successfully"), *ClassName);
        }
        else
        {
            Result.bPassed = false;
            Result.Message = TEXT("Failed to load class");
            UE_LOG(LogBuildIntegration, Error, TEXT("✗ Failed to load %s"), *ClassName);
        }
        
        ValidationResults.Add(Result);
    }
}

void UBuildIntegrationManager::ValidateGameSystems()
{
    // Validar GameMode
    FBuildValidationResult GameModeResult;
    GameModeResult.TestName = TEXT("GameMode Validation");
    GameModeResult.Category = EValidationCategory::GameSystems;
    
    if (GetWorld() && GetWorld()->GetAuthGameMode())
    {
        GameModeResult.bPassed = true;
        GameModeResult.Message = TEXT("GameMode is active");
        UE_LOG(LogBuildIntegration, Log, TEXT("✓ GameMode validation passed"));
    }
    else
    {
        GameModeResult.bPassed = false;
        GameModeResult.Message = TEXT("No active GameMode found");
        UE_LOG(LogBuildIntegration, Warning, TEXT("⚠ No active GameMode found"));
    }
    
    ValidationResults.Add(GameModeResult);
    
    // Validar GameInstance
    FBuildValidationResult GameInstanceResult;
    GameInstanceResult.TestName = TEXT("GameInstance Validation");
    GameInstanceResult.Category = EValidationCategory::GameSystems;
    
    if (GetWorld() && GetWorld()->GetGameInstance())
    {
        GameInstanceResult.bPassed = true;
        GameInstanceResult.Message = TEXT("GameInstance is active");
        UE_LOG(LogBuildIntegration, Log, TEXT("✓ GameInstance validation passed"));
    }
    else
    {
        GameInstanceResult.bPassed = false;
        GameInstanceResult.Message = TEXT("No GameInstance found");
        UE_LOG(LogBuildIntegration, Error, TEXT("✗ No GameInstance found"));
    }
    
    ValidationResults.Add(GameInstanceResult);
}

void UBuildIntegrationManager::ValidateCriticalAssets()
{
    // Validar mapa principal
    FBuildValidationResult MapResult;
    MapResult.TestName = TEXT("MinPlayableMap Validation");
    MapResult.Category = EValidationCategory::Assets;
    
    FString CurrentMapName = GetWorld()->GetMapName();
    if (CurrentMapName.Contains(TEXT("MinPlayableMap")) || CurrentMapName.Contains(TEXT("Untitled")))
    {
        MapResult.bPassed = true;
        MapResult.Message = FString::Printf(TEXT("Map loaded: %s"), *CurrentMapName);
        UE_LOG(LogBuildIntegration, Log, TEXT("✓ Map validation passed: %s"), *CurrentMapName);
    }
    else
    {
        MapResult.bPassed = false;
        MapResult.Message = FString::Printf(TEXT("Unexpected map: %s"), *CurrentMapName);
        UE_LOG(LogBuildIntegration, Warning, TEXT("⚠ Unexpected map: %s"), *CurrentMapName);
    }
    
    ValidationResults.Add(MapResult);
    
    // Contar actors no nível
    FBuildValidationResult ActorCountResult;
    ActorCountResult.TestName = TEXT("Actor Count Validation");
    ActorCountResult.Category = EValidationCategory::Assets;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    if (AllActors.Num() > 5) // Pelo menos alguns actors básicos
    {
        ActorCountResult.bPassed = true;
        ActorCountResult.Message = FString::Printf(TEXT("%d actors in level"), AllActors.Num());
        UE_LOG(LogBuildIntegration, Log, TEXT("✓ Actor count validation passed: %d actors"), AllActors.Num());
    }
    else
    {
        ActorCountResult.bPassed = false;
        ActorCountResult.Message = FString::Printf(TEXT("Too few actors: %d"), AllActors.Num());
        UE_LOG(LogBuildIntegration, Warning, TEXT("⚠ Too few actors in level: %d"), AllActors.Num());
    }
    
    ValidationResults.Add(ActorCountResult);
}

void UBuildIntegrationManager::ValidateModuleConfiguration()
{
    FBuildValidationResult ModuleResult;
    ModuleResult.TestName = TEXT("Module Configuration");
    ModuleResult.Category = EValidationCategory::ModuleConfig;
    
    // Verificar se o módulo TranspersonalGame está carregado
    FModuleManager& ModuleManager = FModuleManager::Get();
    if (ModuleManager.IsModuleLoaded(TEXT("TranspersonalGame")))
    {
        ModuleResult.bPassed = true;
        ModuleResult.Message = TEXT("TranspersonalGame module loaded");
        UE_LOG(LogBuildIntegration, Log, TEXT("✓ TranspersonalGame module is loaded"));
    }
    else
    {
        ModuleResult.bPassed = false;
        ModuleResult.Message = TEXT("TranspersonalGame module not loaded");
        UE_LOG(LogBuildIntegration, Error, TEXT("✗ TranspersonalGame module not loaded"));
    }
    
    ValidationResults.Add(ModuleResult);
}

void UBuildIntegrationManager::DetermineFinalBuildStatus()
{
    int32 PassedTests = 0;
    int32 TotalTests = ValidationResults.Num();
    
    for (const FBuildValidationResult& Result : ValidationResults)
    {
        if (Result.bPassed)
        {
            PassedTests++;
        }
    }
    
    float PassRate = TotalTests > 0 ? (float)PassedTests / (float)TotalTests : 0.0f;
    
    if (PassRate >= 0.9f) // 90% ou mais
    {
        CurrentBuildStatus = EBuildStatus::Healthy;
    }
    else if (PassRate >= 0.7f) // 70-89%
    {
        CurrentBuildStatus = EBuildStatus::Warning;
    }
    else if (PassRate >= 0.5f) // 50-69%
    {
        CurrentBuildStatus = EBuildStatus::Error;
    }
    else // Menos de 50%
    {
        CurrentBuildStatus = EBuildStatus::Critical;
    }
    
    LastBuildTime = FDateTime::Now();
    
    UE_LOG(LogBuildIntegration, Warning, TEXT("Build Status: %s (%d/%d tests passed, %.1f%%)"), 
           *UEnum::GetValueAsString(CurrentBuildStatus), 
           PassedTests, 
           TotalTests, 
           PassRate * 100.0f);
}

void UBuildIntegrationManager::LogValidationResults()
{
    UE_LOG(LogBuildIntegration, Warning, TEXT("=== BUILD VALIDATION RESULTS ==="));
    
    for (const FBuildValidationResult& Result : ValidationResults)
    {
        FString StatusIcon = Result.bPassed ? TEXT("✓") : TEXT("✗");
        FString CategoryStr = UEnum::GetValueAsString(Result.Category);
        
        UE_LOG(LogBuildIntegration, Warning, TEXT("%s [%s] %s: %s"), 
               *StatusIcon, 
               *CategoryStr, 
               *Result.TestName, 
               *Result.Message);
    }
    
    UE_LOG(LogBuildIntegration, Warning, TEXT("=== END VALIDATION RESULTS ==="));
}

EBuildStatus UBuildIntegrationManager::GetCurrentBuildStatus() const
{
    return CurrentBuildStatus;
}

FDateTime UBuildIntegrationManager::GetLastBuildTime() const
{
    return LastBuildTime;
}

FString UBuildIntegrationManager::GetBuildVersion() const
{
    return BuildVersion;
}

TArray<FBuildValidationResult> UBuildIntegrationManager::GetValidationResults() const
{
    return ValidationResults;
}

void UBuildIntegrationManager::SetValidationInterval(float NewInterval)
{
    ValidationInterval = FMath::Max(0.0f, NewInterval);
}

void UBuildIntegrationManager::TriggerManualValidation()
{
    UE_LOG(LogBuildIntegration, Warning, TEXT("Manual validation triggered"));
    PerformFullValidation();
}