#include "VFXManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/StaticMeshComponent.h"

AVFXManager::AVFXManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update LOD 10 times per second
    
    // Valores padrão otimizados para o jogo jurássico
    MaxActiveEffects = 50;
    TargetFrameTime = 16.67f;
    bLowPerformanceMode = false;
    ActiveEffectCount = 0;
    CurrentFrameTime = 0.0f;
}

void AVFXManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Encontrar referência do jogador
    PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    
    // Pré-alocar pool de componentes
    for (int32 i = 0; i < MaxActiveEffects; i++)
    {
        UNiagaraComponent* Component = CreateDefaultSubobject<UNiagaraComponent>(
            *FString::Printf(TEXT("PooledVFX_%d"), i));
        Component->SetAutoDestroy(false);
        Component->SetVisibility(false);
        AvailableComponents.Add(Component);
    }
    
    // Carregar biblioteca de efeitos padrão
    InitializeVFXLibrary();
}

void AVFXManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Track performance
    CurrentFrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    
    // Update LOD system
    UpdateVFXLOD();
    
    // Performance management
    if (CurrentFrameTime > TargetFrameTime * 1.2f) // 20% tolerance
    {
        SetPerformanceMode(true);
    }
    else if (CurrentFrameTime < TargetFrameTime * 0.8f && bLowPerformanceMode)
    {
        SetPerformanceMode(false);
    }
    
    // Cull distant effects
    CullDistantEffects();
}

UNiagaraComponent* AVFXManager::SpawnVFX(const FString& EffectName, 
                                        const FVector& Location, 
                                        const FRotator& Rotation,
                                        const FVector& Scale,
                                        AActor* AttachToActor)
{
    // Verificar se o efeito existe na biblioteca
    if (!VFXLibrary.Contains(EffectName))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Effect not found: %s"), *EffectName);
        return nullptr;
    }
    
    const FVFXDefinition& VFXDef = VFXLibrary[EffectName];
    
    // Verificar limite de instâncias
    int32 CurrentInstances = 0;
    for (UNiagaraComponent* Comp : ActiveComponents)
    {
        if (Comp && Comp->GetAsset() == VFXDef.NiagaraSystem.LoadSynchronous())
        {
            CurrentInstances++;
        }
    }
    
    if (CurrentInstances >= VFXDef.MaxInstances)
    {
        return nullptr; // Limit reached
    }
    
    // Calcular LOD baseado na distância
    float Distance = 0.0f;
    if (PlayerPawn)
    {
        Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Location);
    }
    
    // Verificar se está dentro da distância máxima
    if (Distance > VFXDef.MaxDistance)
    {
        return nullptr;
    }
    
    // Obter componente do pool
    UNiagaraComponent* Component = GetPooledComponent();
    if (!Component)
    {
        return nullptr; // Pool exhausted
    }
    
    // Configurar o efeito
    UNiagaraSystem* NiagaraSystem = VFXDef.NiagaraSystem.LoadSynchronous();
    if (!NiagaraSystem)
    {
        ReturnComponentToPool(Component);
        return nullptr;
    }
    
    Component->SetAsset(NiagaraSystem);
    Component->SetWorldLocationAndRotation(Location, Rotation);
    Component->SetWorldScale3D(Scale);
    Component->SetVisibility(true);
    
    // Aplicar LOD
    if (VFXDef.bRequiresLOD)
    {
        int32 LODLevel = GetLODLevel(Distance);
        Component->SetIntParameter(TEXT("LOD_Level"), LODLevel);
        
        // Ajustar qualidade baseado no LOD
        switch (LODLevel)
        {
            case 0: // High quality
                Component->SetFloatParameter(TEXT("Quality_Multiplier"), 1.0f);
                break;
            case 1: // Medium quality
                Component->SetFloatParameter(TEXT("Quality_Multiplier"), 0.7f);
                break;
            case 2: // Low quality
                Component->SetFloatParameter(TEXT("Quality_Multiplier"), 0.4f);
                break;
            default:
                Component->SetFloatParameter(TEXT("Quality_Multiplier"), 0.2f);
                break;
        }
    }
    
    // Attach se necessário
    if (AttachToActor)
    {
        Component->AttachToComponent(AttachToActor->GetRootComponent(),
                                   FAttachmentTransformRules::KeepWorldTransform);
    }
    
    // Ativar o efeito
    Component->Activate();
    ActiveComponents.Add(Component);
    ActiveEffectCount++;
    
    return Component;
}

void AVFXManager::UpdateVFXLOD()
{
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (UNiagaraComponent* Component : ActiveComponents)
    {
        if (!Component || !Component->IsActive()) continue;
        
        float Distance = FVector::Dist(PlayerLocation, Component->GetComponentLocation());
        int32 NewLODLevel = GetLODLevel(Distance);
        
        // Update LOD parameters
        Component->SetIntParameter(TEXT("LOD_Level"), NewLODLevel);
        
        // Adjust visibility based on performance mode
        if (bLowPerformanceMode && NewLODLevel > 1)
        {
            Component->SetVisibility(false);
        }
        else
        {
            Component->SetVisibility(true);
        }
    }
}

float AVFXManager::GetCurrentVFXLoad() const
{
    return (float)ActiveEffectCount / (float)MaxActiveEffects;
}

void AVFXManager::SetPerformanceMode(bool bLowPerformanceMode)
{
    this->bLowPerformanceMode = bLowPerformanceMode;
    
    if (bLowPerformanceMode)
    {
        // Reduzir qualidade geral
        for (UNiagaraComponent* Component : ActiveComponents)
        {
            if (Component && Component->IsActive())
            {
                Component->SetFloatParameter(TEXT("Performance_Scale"), 0.5f);
            }
        }
    }
    else
    {
        // Restaurar qualidade
        for (UNiagaraComponent* Component : ActiveComponents)
        {
            if (Component && Component->IsActive())
            {
                Component->SetFloatParameter(TEXT("Performance_Scale"), 1.0f);
            }
        }
    }
}

void AVFXManager::PlayDinosaurFootstep(const FVector& Location, float DinosaurSize)
{
    FString EffectName;
    
    // Selecionar efeito baseado no tamanho
    if (DinosaurSize > 10.0f) // Grandes predadores
    {
        EffectName = TEXT("Footstep_Large");
    }
    else if (DinosaurSize > 3.0f) // Dinossauros médios
    {
        EffectName = TEXT("Footstep_Medium");
    }
    else // Pequenos dinossauros
    {
        EffectName = TEXT("Footstep_Small");
    }
    
    UNiagaraComponent* Effect = SpawnVFX(EffectName, Location);
    if (Effect)
    {
        Effect->SetFloatParameter(TEXT("Size_Multiplier"), DinosaurSize);
    }
}

void AVFXManager::PlayBloodSplatter(const FVector& Location, const FVector& Direction, float Intensity)
{
    UNiagaraComponent* Effect = SpawnVFX(TEXT("Blood_Splatter"), Location);
    if (Effect)
    {
        Effect->SetVectorParameter(TEXT("Impact_Direction"), Direction);
        Effect->SetFloatParameter(TEXT("Intensity"), Intensity);
    }
}

void AVFXManager::PlayEnvironmentalDisturbance(const FVector& Location, float Radius)
{
    UNiagaraComponent* Effect = SpawnVFX(TEXT("Environment_Disturbance"), Location);
    if (Effect)
    {
        Effect->SetFloatParameter(TEXT("Disturbance_Radius"), Radius);
    }
}

UNiagaraComponent* AVFXManager::GetPooledComponent()
{
    if (AvailableComponents.Num() > 0)
    {
        UNiagaraComponent* Component = AvailableComponents.Pop();
        return Component;
    }
    
    return nullptr; // Pool is empty
}

void AVFXManager::ReturnComponentToPool(UNiagaraComponent* Component)
{
    if (!Component) return;
    
    Component->Deactivate();
    Component->SetVisibility(false);
    Component->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
    
    ActiveComponents.Remove(Component);
    AvailableComponents.Add(Component);
    ActiveEffectCount--;
}

int32 AVFXManager::GetLODLevel(float Distance) const
{
    if (Distance <= LODDistance_High)
        return 0; // High quality
    else if (Distance <= LODDistance_Medium)
        return 1; // Medium quality
    else if (Distance <= LODDistance_Low)
        return 2; // Low quality
    else
        return 3; // Minimal quality
}

void AVFXManager::CullDistantEffects()
{
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (int32 i = ActiveComponents.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Component = ActiveComponents[i];
        if (!Component) continue;
        
        float Distance = FVector::Dist(PlayerLocation, Component->GetComponentLocation());
        
        // Cull effects that are too far or finished
        if (Distance > LODDistance_Low * 1.5f || !Component->IsActive())
        {
            ReturnComponentToPool(Component);
        }
    }
}

void AVFXManager::InitializeVFXLibrary()
{
    // Efeitos de pegadas
    FVFXDefinition FootstepLarge;
    FootstepLarge.EffectName = TEXT("Footstep_Large");
    FootstepLarge.Priority = EVFXPriority::High;
    FootstepLarge.Category = EVFXCategory::Creature;
    FootstepLarge.MaxDistance = 3000.0f;
    FootstepLarge.MaxInstances = 5;
    VFXLibrary.Add(FootstepLarge.EffectName, FootstepLarge);
    
    FVFXDefinition FootstepMedium;
    FootstepMedium.EffectName = TEXT("Footstep_Medium");
    FootstepMedium.Priority = EVFXPriority::Medium;
    FootstepMedium.Category = EVFXCategory::Creature;
    FootstepMedium.MaxDistance = 2000.0f;
    FootstepMedium.MaxInstances = 8;
    VFXLibrary.Add(FootstepMedium.EffectName, FootstepMedium);
    
    FVFXDefinition FootstepSmall;
    FootstepSmall.EffectName = TEXT("Footstep_Small");
    FootstepSmall.Priority = EVFXPriority::Low;
    FootstepSmall.Category = EVFXCategory::Creature;
    FootstepSmall.MaxDistance = 1000.0f;
    FootstepSmall.MaxInstances = 15;
    VFXLibrary.Add(FootstepSmall.EffectName, FootstepSmall);
    
    // Efeitos de sangue
    FVFXDefinition BloodSplatter;
    BloodSplatter.EffectName = TEXT("Blood_Splatter");
    BloodSplatter.Priority = EVFXPriority::Critical;
    BloodSplatter.Category = EVFXCategory::Combat;
    BloodSplatter.MaxDistance = 5000.0f;
    BloodSplatter.MaxInstances = 10;
    VFXLibrary.Add(BloodSplatter.EffectName, BloodSplatter);
    
    // Efeitos ambientais
    FVFXDefinition EnvDisturbance;
    EnvDisturbance.EffectName = TEXT("Environment_Disturbance");
    EnvDisturbance.Priority = EVFXPriority::Medium;
    EnvDisturbance.Category = EVFXCategory::Environmental;
    EnvDisturbance.MaxDistance = 4000.0f;
    EnvDisturbance.MaxInstances = 12;
    VFXLibrary.Add(EnvDisturbance.EffectName, EnvDisturbance);
}