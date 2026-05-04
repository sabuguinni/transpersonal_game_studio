#include "BuildIntegrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "Engine/GameViewportClient.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Atmosphere/AtmosphericFog.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/SkyLight.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/ExponentialHeightFog.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Verificar a cada 5 segundos
    
    // Inicializar contadores
    LastActorCount = 0;
    LastCompilationCheck = 0.0f;
    CompilationCheckInterval = 30.0f; // Verificar compilação a cada 30 segundos
    
    // Configurar tipos críticos que devem ter apenas 1 instância
    CriticalSingletonTypes.Add(TEXT("DirectionalLight"));
    CriticalSingletonTypes.Add(TEXT("SkyAtmosphere"));
    CriticalSingletonTypes.Add(TEXT("SkyLight"));
    CriticalSingletonTypes.Add(TEXT("ExponentialHeightFog"));
    
    // Configurar tipos que devem existir no mapa
    RequiredActorTypes.Add(TEXT("PlayerStart"));
    RequiredActorTypes.Add(TEXT("Landscape"));
    RequiredActorTypes.Add(TEXT("DirectionalLight"));
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Inicializado"));
}

void UBuildIntegrationManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Executar verificação inicial
    PerformIntegrationCheck();
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: BeginPlay executado"));
}

void UBuildIntegrationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Verificar se é hora de fazer uma verificação de integração
    LastCompilationCheck += DeltaTime;
    if (LastCompilationCheck >= CompilationCheckInterval)
    {
        PerformIntegrationCheck();
        LastCompilationCheck = 0.0f;
    }
}

void UBuildIntegrationManager::PerformIntegrationCheck()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: Mundo não disponível"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Executando verificação de integração"));
    
    // Verificar actores no mundo
    CheckWorldActors();
    
    // Verificar duplicados críticos
    CheckCriticalDuplicates();
    
    // Verificar actores obrigatórios
    CheckRequiredActors();
    
    // Gerar relatório
    GenerateIntegrationReport();
}

void UBuildIntegrationManager::CheckWorldActors()
{
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    int32 CurrentActorCount = AllActors.Num();
    
    if (CurrentActorCount != LastActorCount)
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Contagem de actores mudou de %d para %d"), 
               LastActorCount, CurrentActorCount);
        LastActorCount = CurrentActorCount;
    }
    
    // Contar por tipo
    ActorTypeCounts.Empty();
    for (AActor* Actor : AllActors)
    {
        if (Actor)
        {
            FString ActorType = Actor->GetClass()->GetName();
            int32* Count = ActorTypeCounts.Find(ActorType);
            if (Count)
            {
                (*Count)++;
            }
            else
            {
                ActorTypeCounts.Add(ActorType, 1);
            }
        }
    }
}

void UBuildIntegrationManager::CheckCriticalDuplicates()
{
    TArray<FString> ProblemsFound;
    
    for (const FString& CriticalType : CriticalSingletonTypes)
    {
        int32* Count = ActorTypeCounts.Find(CriticalType);
        if (Count && *Count > 1)
        {
            FString Problem = FString::Printf(TEXT("DUPLICADO CRÍTICO: %s tem %d instâncias (deveria ter 1)"), 
                                            *CriticalType, *Count);
            ProblemsFound.Add(Problem);
            UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: %s"), *Problem);
        }
    }
    
    // Armazenar problemas encontrados
    CriticalProblems = ProblemsFound;
}

void UBuildIntegrationManager::CheckRequiredActors()
{
    TArray<FString> MissingActors;
    
    for (const FString& RequiredType : RequiredActorTypes)
    {
        int32* Count = ActorTypeCounts.Find(RequiredType);
        if (!Count || *Count == 0)
        {
            FString Missing = FString::Printf(TEXT("ACTOR OBRIGATÓRIO AUSENTE: %s"), *RequiredType);
            MissingActors.Add(Missing);
            UE_LOG(LogTemp, Error, TEXT("BuildIntegrationManager: %s"), *Missing);
        }
    }
    
    // Armazenar actores em falta
    MissingRequiredActors = MissingActors;
}

void UBuildIntegrationManager::GenerateIntegrationReport()
{
    FString Report = TEXT("=== RELATÓRIO DE INTEGRAÇÃO ===\n");
    Report += FString::Printf(TEXT("Timestamp: %s\n"), *FDateTime::Now().ToString());
    Report += FString::Printf(TEXT("Total de actores: %d\n\n"), LastActorCount);
    
    // Adicionar contagem por tipo
    Report += TEXT("CONTAGEM POR TIPO:\n");
    for (const auto& Pair : ActorTypeCounts)
    {
        Report += FString::Printf(TEXT("  %s: %d\n"), *Pair.Key, Pair.Value);
    }
    
    // Adicionar problemas críticos
    if (CriticalProblems.Num() > 0)
    {
        Report += TEXT("\nPROBLEMAS CRÍTICOS:\n");
        for (const FString& Problem : CriticalProblems)
        {
            Report += FString::Printf(TEXT("  ❌ %s\n"), *Problem);
        }
    }
    
    // Adicionar actores em falta
    if (MissingRequiredActors.Num() > 0)
    {
        Report += TEXT("\nACTORES OBRIGATÓRIOS EM FALTA:\n");
        for (const FString& Missing : MissingRequiredActors)
        {
            Report += FString::Printf(TEXT("  ⚠️ %s\n"), *Missing);
        }
    }
    
    // Status geral
    bool HasCriticalIssues = (CriticalProblems.Num() > 0) || (MissingRequiredActors.Num() > 0);
    if (HasCriticalIssues)
    {
        Report += TEXT("\n🔴 STATUS: PROBLEMAS CRÍTICOS DETECTADOS\n");
    }
    else
    {
        Report += TEXT("\n🟢 STATUS: INTEGRAÇÃO OK\n");
    }
    
    // Log do relatório
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager Report:\n%s"), *Report);
    
    // Armazenar relatório
    LastIntegrationReport = Report;
}

FString UBuildIntegrationManager::GetLastIntegrationReport() const
{
    return LastIntegrationReport;
}

bool UBuildIntegrationManager::HasCriticalIssues() const
{
    return (CriticalProblems.Num() > 0) || (MissingRequiredActors.Num() > 0);
}

void UBuildIntegrationManager::ForceIntegrationCheck()
{
    PerformIntegrationCheck();
}

TMap<FString, int32> UBuildIntegrationManager::GetActorTypeCounts() const
{
    return ActorTypeCounts;
}