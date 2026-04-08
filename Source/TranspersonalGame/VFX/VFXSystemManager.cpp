#include "VFXSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/StaticMeshComponent.h"

AVFXSystemManager::AVFXSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10fps para performance

    // Configuração inicial do pool
    MaxActiveEffects = 100;
    LODUpdateInterval = 0.5f;
    MaxVFXBudgetMS = 2.0f;
}

void AVFXSystemManager::BeginPlay()
{
    Super::BeginPlay();

    // Pre-populate o pool com componentes
    for (int32 i = 0; i < 20; ++i)
    {
        UNiagaraComponent* Component = CreateDefaultSubobject<UNiagaraComponent>(FName(*FString::Printf(TEXT("PooledVFX_%d"), i)));
        Component->SetAutoDestroy(false);
        Component->SetActive(false);
        AvailableComponents.Add(Component);
    }

    // Registar efeitos base do jogo
    RegisterDefaultEffects();

    UE_LOG(LogTemp, Log, TEXT("VFX System Manager initialized with %d pooled components"), AvailableComponents.Num());
}

void AVFXSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update LOD system
    LastLODUpdate += DeltaTime;
    if (LastLODUpdate >= LODUpdateInterval)
    {
        UpdateVFXLOD();
        LastLODUpdate = 0.0f;
    }

    // Cleanup finished effects
    CleanupFinishedEffects();

    // Performance monitoring
    CurrentFrameVFXTime = 0.0f; // Reset counter for next frame
}

UNiagaraComponent* AVFXSystemManager::SpawnVFXEffect(FName EffectName, FVector Location, FRotator Rotation, AActor* AttachToActor)
{
    // Verificar se o efeito está registado
    FVFXDefinition* EffectDef = RegisteredEffects.Find(EffectName);
    if (!EffectDef)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Effect '%s' not found in registered effects"), *EffectName.ToString());
        return nullptr;
    }

    // Verificar limites de performance
    if (ActiveComponents.Num() >= MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("Max VFX effects reached (%d), skipping spawn of '%s'"), MaxActiveEffects, *EffectName.ToString());
        return nullptr;
    }

    // Verificar distância ao jogador para LOD
    float DistanceToPlayer = CalculateDistanceToPlayer(Location);
    if (DistanceToPlayer > EffectDef->MaxDistance)
    {
        return nullptr; // Muito longe para renderizar
    }

    // Obter componente do pool
    UNiagaraComponent* Component = GetPooledComponent();
    if (!Component)
    {
        UE_LOG(LogTemp, Warning, TEXT("No available VFX components in pool"));
        return nullptr;
    }

    // Configurar o efeito
    Component->SetAsset(EffectDef->NiagaraSystem.LoadSynchronous());
    Component->SetWorldLocation(Location);
    Component->SetWorldRotation(Rotation);

    // Anexar a actor se especificado
    if (AttachToActor)
    {
        Component->AttachToComponent(AttachToActor->GetRootComponent(), 
            FAttachmentTransformRules::KeepWorldTransform);
    }

    // Configurar LOD baseado na distância
    int32 LODLevel = GetLODLevelForDistance(DistanceToPlayer);
    Component->SetUserVariableInt(FName("LOD_Level"), LODLevel);

    // Activar o efeito
    Component->SetActive(true);
    Component->ResetSystem();

    ActiveComponents.Add(Component);

    UE_LOG(LogTemp, Log, TEXT("Spawned VFX '%s' at distance %.2f with LOD %d"), 
        *EffectName.ToString(), DistanceToPlayer, LODLevel);

    return Component;
}

void AVFXSystemManager::StopVFXEffect(UNiagaraComponent* Effect)
{
    if (!Effect) return;

    Effect->SetActive(false);
    ReturnComponentToPool(Effect);
}

void AVFXSystemManager::RegisterEffect(const FVFXDefinition& EffectDefinition)
{
    RegisteredEffects.Add(EffectDefinition.EffectName, EffectDefinition);
    UE_LOG(LogTemp, Log, TEXT("Registered VFX effect: %s"), *EffectDefinition.EffectName.ToString());
}

void AVFXSystemManager::UpdateVFXLOD()
{
    for (UNiagaraComponent* Component : ActiveComponents)
    {
        if (!Component || !Component->IsActive()) continue;

        float Distance = CalculateDistanceToPlayer(Component->GetComponentLocation());
        int32 NewLODLevel = GetLODLevelForDistance(Distance);
        
        // Update LOD se mudou
        int32 CurrentLOD = Component->GetUserVariableInt(FName("LOD_Level"));
        if (CurrentLOD != NewLODLevel)
        {
            Component->SetUserVariableInt(FName("LOD_Level"), NewLODLevel);
        }

        // Desactivar se muito longe
        FVFXDefinition* EffectDef = nullptr;
        for (auto& Pair : RegisteredEffects)
        {
            if (Pair.Value.NiagaraSystem.Get() == Component->GetAsset())
            {
                EffectDef = &Pair.Value;
                break;
            }
        }

        if (EffectDef && Distance > EffectDef->MaxDistance)
        {
            StopVFXEffect(Component);
        }
    }
}

void AVFXSystemManager::SetVFXQualityLevel(int32 QualityLevel)
{
    // Ajustar limites baseados na qualidade
    switch (QualityLevel)
    {
        case 0: // Low
            MaxActiveEffects = 50;
            MaxVFXBudgetMS = 1.0f;
            LODUpdateInterval = 1.0f;
            break;
        case 1: // Medium
            MaxActiveEffects = 75;
            MaxVFXBudgetMS = 1.5f;
            LODUpdateInterval = 0.75f;
            break;
        case 2: // High
            MaxActiveEffects = 100;
            MaxVFXBudgetMS = 2.0f;
            LODUpdateInterval = 0.5f;
            break;
        case 3: // Ultra
            MaxActiveEffects = 150;
            MaxVFXBudgetMS = 3.0f;
            LODUpdateInterval = 0.25f;
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("VFX Quality set to level %d - Max Effects: %d, Budget: %.1fms"), 
        QualityLevel, MaxActiveEffects, MaxVFXBudgetMS);
}

UNiagaraComponent* AVFXSystemManager::GetPooledComponent()
{
    if (AvailableComponents.Num() > 0)
    {
        UNiagaraComponent* Component = AvailableComponents.Pop();
        return Component;
    }

    // Se não há componentes disponíveis, criar um novo
    UNiagaraComponent* NewComponent = NewObject<UNiagaraComponent>(this);
    NewComponent->SetAutoDestroy(false);
    return NewComponent;
}

void AVFXSystemManager::ReturnComponentToPool(UNiagaraComponent* Component)
{
    if (!Component) return;

    // Limpar estado
    Component->SetActive(false);
    Component->SetAsset(nullptr);
    Component->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

    // Remover da lista activa
    ActiveComponents.Remove(Component);

    // Retornar ao pool se há espaço
    if (AvailableComponents.Num() < 50) // Limite do pool
    {
        AvailableComponents.Add(Component);
    }
    else
    {
        // Destruir se o pool está cheio
        Component->DestroyComponent();
    }
}

void AVFXSystemManager::CleanupFinishedEffects()
{
    for (int32 i = ActiveComponents.Num() - 1; i >= 0; --i)
    {
        UNiagaraComponent* Component = ActiveComponents[i];
        if (!Component || !Component->IsActive() || Component->IsComplete())
        {
            ReturnComponentToPool(Component);
        }
    }
}

float AVFXSystemManager::CalculateDistanceToPlayer(const FVector& EffectLocation) const
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || !PC->GetPawn()) return 99999.0f;

    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    return FVector::Dist(PlayerLocation, EffectLocation);
}

int32 AVFXSystemManager::GetLODLevelForDistance(float Distance) const
{
    // Sistema LOD de 3 níveis
    if (Distance < 1000.0f) return 0;      // High quality
    else if (Distance < 3000.0f) return 1; // Medium quality  
    else return 2;                          // Low quality
}

void AVFXSystemManager::RegisterDefaultEffects()
{
    // Efeitos atmosféricos base
    FVFXDefinition AtmosphericDust;
    AtmosphericDust.EffectName = FName("Atmospheric_Dust");
    AtmosphericDust.Category = EVFXCategory::Atmospheric;
    AtmosphericDust.Intensity = EVFXIntensity::Subtle;
    AtmosphericDust.MaxDistance = 2000.0f;
    AtmosphericDust.MaxInstances = 20;
    RegisterEffect(AtmosphericDust);

    // Efeitos de criaturas
    FVFXDefinition CreatureBreath;
    CreatureBreath.EffectName = FName("Creature_Breath");
    CreatureBreath.Category = EVFXCategory::Creature;
    CreatureBreath.Intensity = EVFXIntensity::Moderate;
    CreatureBreath.MaxDistance = 1500.0f;
    CreatureBreath.MaxInstances = 10;
    RegisterEffect(CreatureBreath);

    // Efeitos de sobrevivência
    FVFXDefinition CampfireSmoke;
    CampfireSmoke.EffectName = FName("Survival_CampfireSmoke");
    CampfireSmoke.Category = EVFXCategory::Survival;
    CampfireSmoke.Intensity = EVFXIntensity::Moderate;
    CampfireSmoke.MaxDistance = 3000.0f;
    CampfireSmoke.MaxInstances = 5;
    CampfireSmoke.bAffectsGameplay = true; // Fumo pode atrair predadores
    RegisterEffect(CampfireSmoke);

    UE_LOG(LogTemp, Log, TEXT("Registered %d default VFX effects"), RegisteredEffects.Num());
}

void AVFXSystemManager::ShowVFXDebugInfo(bool bShow)
{
    if (bShow)
    {
        UE_LOG(LogTemp, Log, TEXT("=== VFX DEBUG INFO ==="));
        UE_LOG(LogTemp, Log, TEXT("Active Effects: %d/%d"), ActiveComponents.Num(), MaxActiveEffects);
        UE_LOG(LogTemp, Log, TEXT("Available Pool: %d"), AvailableComponents.Num());
        UE_LOG(LogTemp, Log, TEXT("Registered Effects: %d"), RegisteredEffects.Num());
        UE_LOG(LogTemp, Log, TEXT("Current Frame VFX Time: %.2fms"), CurrentFrameVFXTime);
    }
}