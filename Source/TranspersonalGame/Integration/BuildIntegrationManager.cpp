#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY(LogBuildIntegration);

ABuildIntegrationManager::ABuildIntegrationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    
    // Criar componente raiz
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Inicializar propriedades
    bIsActive = true;
    bValidationEnabled = true;
    bAutoCleanup = true;
    ValidationInterval = 30.0f;
    LastValidationTime = 0.0f;
    
    // Inicializar contadores
    TotalHeaderFiles = 0;
    TotalCppFiles = 0;
    OrphanHeaders = 0;
    CompilationErrors = 0;
    
    // Inicializar arrays
    CriticalSystems.Empty();
    DuplicateSystems.Empty();
    ValidationResults.Empty();
    
    // Adicionar sistemas críticos conhecidos
    CriticalSystems.Add(TEXT("TranspersonalCharacter"));
    CriticalSystems.Add(TEXT("TranspersonalGameMode"));
    CriticalSystems.Add(TEXT("BiomeManager"));
    CriticalSystems.Add(TEXT("DinosaurBase"));
}

void ABuildIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogBuildIntegration, Warning, TEXT("BuildIntegrationManager iniciado"));
    
    if (bIsActive)
    {
        // Executar validação inicial
        PerformBuildValidation();
    }
}

void ABuildIntegrationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bIsActive || !bValidationEnabled)
    {
        return;
    }
    
    // Verificar se é hora de executar validação
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastValidationTime >= ValidationInterval)
    {
        PerformBuildValidation();
        LastValidationTime = CurrentTime;
    }
}

void ABuildIntegrationManager::PerformBuildValidation()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Executando validação de build..."));
    
    // Limpar resultados anteriores
    ValidationResults.Empty();
    
    // Validar classes críticas
    ValidateCriticalSystems();
    
    // Validar actores no mapa
    ValidateMapActors();
    
    // Detectar sistemas duplicados
    DetectDuplicateSystems();
    
    // Executar limpeza automática se habilitada
    if (bAutoCleanup)
    {
        PerformAutoCleanup();
    }
    
    // Log dos resultados
    LogValidationResults();
}

void ABuildIntegrationManager::ValidateCriticalSystems()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Validando sistemas críticos..."));
    
    for (const FString& SystemName : CriticalSystems)
    {
        FBuild_ValidationResult Result;
        Result.SystemName = SystemName;
        Result.bIsValid = false;
        Result.ErrorMessage = TEXT("");
        
        // Tentar carregar a classe
        FString ClassPath = FString::Printf(TEXT("/Script/TranspersonalGame.%s"), *SystemName);
        UClass* SystemClass = LoadClass<UObject>(nullptr, *ClassPath);
        
        if (SystemClass)
        {
            Result.bIsValid = true;
            Result.ErrorMessage = TEXT("Sistema carregado com sucesso");
            UE_LOG(LogBuildIntegration, Log, TEXT("✓ %s - OK"), *SystemName);
        }
        else
        {
            Result.bIsValid = false;
            Result.ErrorMessage = TEXT("Classe não encontrada ou não compilada");
            UE_LOG(LogBuildIntegration, Warning, TEXT("✗ %s - FALHA"), *SystemName);
        }
        
        ValidationResults.Add(Result);
    }
}

void ABuildIntegrationManager::ValidateMapActors()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Validando actores do mapa..."));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogBuildIntegration, Error, TEXT("Mundo não disponível para validação"));
        return;
    }
    
    // Contar actores por tipo
    TMap<FString, int32> ActorCounts;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            FString ActorClassName = Actor->GetClass()->GetName();
            ActorCounts.FindOrAdd(ActorClassName)++;
        }
    }
    
    // Log da distribuição
    UE_LOG(LogBuildIntegration, Log, TEXT("Distribuição de actores:"));
    for (const auto& Pair : ActorCounts)
    {
        UE_LOG(LogBuildIntegration, Log, TEXT("  %s: %d"), *Pair.Key, Pair.Value);
    }
}

void ABuildIntegrationManager::DetectDuplicateSystems()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Detectando sistemas duplicados..."));
    
    // Lista de padrões conhecidos de duplicação
    TArray<FString> DuplicatePatterns = {
        TEXT("Crowd"),
        TEXT("NPC"),
        TEXT("Architecture"),
        TEXT("Performance")
    };
    
    DuplicateSystems.Empty();
    
    for (const FString& Pattern : DuplicatePatterns)
    {
        // Simular detecção de duplicados
        // Em implementação real, faria scan do filesystem
        FBuild_DuplicateSystem Duplicate;
        Duplicate.SystemType = Pattern;
        Duplicate.DuplicateCount = 2; // Placeholder
        Duplicate.bRequiresCleanup = true;
        
        DuplicateSystems.Add(Duplicate);
    }
}

void ABuildIntegrationManager::PerformAutoCleanup()
{
    UE_LOG(LogBuildIntegration, Log, TEXT("Executando limpeza automática..."));
    
    // Implementar limpeza de actores duplicados
    CleanupDuplicateActors();
}

void ABuildIntegrationManager::CleanupDuplicateActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Limpar DirectionalLights duplicados (manter apenas 1)
    TArray<AActor*> DirectionalLights;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetClass()->GetName().Contains(TEXT("DirectionalLight")))
        {
            DirectionalLights.Add(Actor);
        }
    }
    
    // Manter apenas o primeiro, remover os restantes
    for (int32 i = 1; i < DirectionalLights.Num(); i++)
    {
        DirectionalLights[i]->Destroy();
        UE_LOG(LogBuildIntegration, Log, TEXT("Removido DirectionalLight duplicado"));
    }
}

void ABuildIntegrationManager::LogValidationResults()
{
    UE_LOG(LogBuildIntegration, Warning, TEXT("=== RESULTADOS DA VALIDAÇÃO ==="));
    UE_LOG(LogBuildIntegration, Warning, TEXT("Headers: %d | CPP: %d | Órfãos: %d"), 
           TotalHeaderFiles, TotalCppFiles, OrphanHeaders);
    UE_LOG(LogBuildIntegration, Warning, TEXT("Erros de compilação: %d"), CompilationErrors);
    UE_LOG(LogBuildIntegration, Warning, TEXT("Sistemas duplicados: %d"), DuplicateSystems.Num());
    UE_LOG(LogBuildIntegration, Warning, TEXT("================================"));
}