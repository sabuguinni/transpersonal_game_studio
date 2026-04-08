#include "VFXSystemManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"

AVFXSystemManager::AVFXSystemManager()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Configuração inicial de performance
    MaxSimultaneousEffects = 50;
    PerformanceBudget_GPU = 8.0f;
    bEnableAdaptiveLOD = true;
}

void AVFXSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Inicializar timer de LOD update
    if (bEnableAdaptiveLOD)
    {
        GetWorld()->GetTimerManager().SetTimer(LODUpdateTimer, this, 
            &AVFXSystemManager::UpdateLODSystem, 0.5f, true);
    }
}

UNiagaraComponent* AVFXSystemManager::SpawnVFXAtLocation(const FString& EffectName, 
                                                        const FVector& Location, 
                                                        const FRotator& Rotation,
                                                        const FVector& Scale)
{
    if (!VFXDatabase)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFXSystemManager: VFXDatabase not set"));
        return nullptr;
    }

    // Buscar definição do efeito
    FVFXDefinition* EffectDef = VFXDatabase->FindRow<FVFXDefinition>(FName(*EffectName), TEXT(""));
    if (!EffectDef)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFXSystemManager: Effect '%s' not found"), *EffectName);
        return nullptr;
    }

    // Verificar budget de performance
    if (ActiveEffects.Num() >= MaxSimultaneousEffects)
    {
        CleanupInactiveEffects();
        
        if (ActiveEffects.Num() >= MaxSimultaneousEffects)
        {
            UE_LOG(LogTemp, Warning, TEXT("VFXSystemManager: Max effects limit reached"));
            return nullptr;
        }
    }

    // Calcular LOD baseado na distância
    int32 LODLevel = CalculateEffectLOD(Location);
    
    // Carregar sistema Niagara
    UNiagaraSystem* NiagaraSystem = EffectDef->NiagaraSystem.LoadSynchronous();
    if (!NiagaraSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("VFXSystemManager: Failed to load Niagara system for '%s'"), *EffectName);
        return nullptr;
    }

    // Spawn do efeito
    UNiagaraComponent* SpawnedEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(), NiagaraSystem, Location, Rotation, Scale, true, true);

    if (SpawnedEffect)
    {
        // Aplicar LOD
        SpawnedEffect->SetVariableFloat(TEXT("LODLevel"), static_cast<float>(LODLevel));
        
        // Configurar parâmetros de performance baseados no LOD
        switch (LODLevel)
        {
        case 0: // High quality
            SpawnedEffect->SetVariableFloat(TEXT("ParticleCount"), 1.0f);
            SpawnedEffect->SetVariableFloat(TEXT("UpdateRate"), 1.0f);
            break;
        case 1: // Medium quality
            SpawnedEffect->SetVariableFloat(TEXT("ParticleCount"), 0.6f);
            SpawnedEffect->SetVariableFloat(TEXT("UpdateRate"), 0.8f);
            break;
        case 2: // Low quality
            SpawnedEffect->SetVariableFloat(TEXT("ParticleCount"), 0.3f);
            SpawnedEffect->SetVariableFloat(TEXT("UpdateRate"), 0.5f);
            break;
        }

        ActiveEffects.Add(SpawnedEffect);
        
        UE_LOG(LogTemp, Log, TEXT("VFXSystemManager: Spawned '%s' at LOD %d"), *EffectName, LODLevel);
    }

    return SpawnedEffect;
}

UNiagaraComponent* AVFXSystemManager::SpawnVFXAttached(const FString& EffectName,
                                                      USceneComponent* AttachComponent,
                                                      const FName& AttachPointName,
                                                      const FVector& Location,
                                                      const FRotator& Rotation,
                                                      const FVector& Scale)
{
    if (!AttachComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFXSystemManager: AttachComponent is null"));
        return nullptr;
    }

    // Usar a localização do componente para calcular LOD
    FVector WorldLocation = AttachComponent->GetComponentLocation() + Location;
    
    UNiagaraComponent* SpawnedEffect = SpawnVFXAtLocation(EffectName, WorldLocation, Rotation, Scale);
    
    if (SpawnedEffect)
    {
        // Anexar ao componente
        SpawnedEffect->AttachToComponent(AttachComponent, 
            FAttachmentTransformRules::KeepRelativeTransform, AttachPointName);
        SpawnedEffect->SetRelativeLocation(Location);
        SpawnedEffect->SetRelativeRotation(Rotation);
        SpawnedEffect->SetRelativeScale3D(Scale);
    }

    return SpawnedEffect;
}

void AVFXSystemManager::ApplyCreatureVariation(UNiagaraComponent* VFXComponent, int32 CreatureID)
{
    if (!VFXComponent)
        return;

    // Gerar variação baseada no ID da criatura
    FRandomStream RandomStream(CreatureID);
    
    // Variações de cor (baseadas no ID para consistência)
    float HueShift = RandomStream.FRandRange(-0.2f, 0.2f);
    float SaturationMultiplier = RandomStream.FRandRange(0.8f, 1.2f);
    float BrightnessMultiplier = RandomStream.FRandRange(0.9f, 1.1f);
    
    VFXComponent->SetVariableFloat(TEXT("HueShift"), HueShift);
    VFXComponent->SetVariableFloat(TEXT("SaturationMult"), SaturationMultiplier);
    VFXComponent->SetVariableFloat(TEXT("BrightnessMult"), BrightnessMultiplier);
    
    // Variações de escala e intensidade
    float ScaleVariation = RandomStream.FRandRange(0.85f, 1.15f);
    float IntensityVariation = RandomStream.FRandRange(0.9f, 1.1f);
    
    VFXComponent->SetVariableFloat(TEXT("ScaleVariation"), ScaleVariation);
    VFXComponent->SetVariableFloat(TEXT("IntensityVariation"), IntensityVariation);
    
    UE_LOG(LogTemp, Log, TEXT("VFXSystemManager: Applied variation for creature ID %d"), CreatureID);
}

float AVFXSystemManager::GetCurrentGPUBudgetUsage() const
{
    // Estimativa baseada no número de efeitos ativos
    // Em implementação real, isso seria conectado ao profiler de GPU
    float EstimatedUsage = (static_cast<float>(ActiveEffects.Num()) / MaxSimultaneousEffects) * PerformanceBudget_GPU;
    return FMath::Clamp(EstimatedUsage, 0.0f, PerformanceBudget_GPU);
}

void AVFXSystemManager::ForceUpdateLOD()
{
    UpdateLODSystem();
}

void AVFXSystemManager::UpdateLODSystem()
{
    if (!bEnableAdaptiveLOD)
        return;

    CleanupInactiveEffects();
    
    // Verificar se estamos excedendo o budget de performance
    float CurrentUsage = GetCurrentGPUBudgetUsage();
    bool bNeedsOptimization = CurrentUsage > (PerformanceBudget_GPU * 0.8f);
    
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (!Effect || !Effect->IsActive())
            continue;
            
        FVector EffectLocation = Effect->GetComponentLocation();
        int32 NewLOD = CalculateEffectLOD(EffectLocation);
        
        // Se precisamos otimizar, forçar LOD mais baixo
        if (bNeedsOptimization)
        {
            NewLOD = FMath::Min(NewLOD + 1, 2);
        }
        
        // Aplicar novo LOD se mudou
        float CurrentLOD = 0.0f;
        if (Effect->GetVariableFloat(TEXT("LODLevel"), CurrentLOD))
        {
            if (FMath::Abs(CurrentLOD - static_cast<float>(NewLOD)) > 0.1f)
            {
                Effect->SetVariableFloat(TEXT("LODLevel"), static_cast<float>(NewLOD));
                
                // Atualizar parâmetros de qualidade
                switch (NewLOD)
                {
                case 0:
                    Effect->SetVariableFloat(TEXT("ParticleCount"), 1.0f);
                    Effect->SetVariableFloat(TEXT("UpdateRate"), 1.0f);
                    break;
                case 1:
                    Effect->SetVariableFloat(TEXT("ParticleCount"), 0.6f);
                    Effect->SetVariableFloat(TEXT("UpdateRate"), 0.8f);
                    break;
                case 2:
                    Effect->SetVariableFloat(TEXT("ParticleCount"), 0.3f);
                    Effect->SetVariableFloat(TEXT("UpdateRate"), 0.5f);
                    break;
                }
            }
        }
    }
}

int32 AVFXSystemManager::CalculateEffectLOD(const FVector& EffectLocation) const
{
    // Obter posição do jogador
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
        return 2; // LOD mais baixo se não conseguir determinar posição do jogador
    
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, EffectLocation);
    
    // Determinar LOD baseado na distância
    if (Distance < 2000.0f)
        return 0; // High quality
    else if (Distance < 5000.0f)
        return 1; // Medium quality
    else
        return 2; // Low quality
}

void AVFXSystemManager::CleanupInactiveEffects()
{
    ActiveEffects.RemoveAll([](UNiagaraComponent* Effect)
    {
        return !Effect || !IsValid(Effect) || !Effect->IsActive();
    });
}