#include "VFXSystemManager.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

AVFXSystemManager::AVFXSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update LOD 10 vezes por segundo
    
    // Inicializar pools
    VFXPool_Environmental.Reserve(50);
    VFXPool_Combat.Reserve(30);
    VFXPool_Atmospheric.Reserve(20);
    
    CurrentFrameTime = 0.0f;
    ActiveVFXCount = 0;
}

void AVFXSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Encontrar referência ao jogador
    PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    
    // Pre-carregar sistemas VFX críticos
    if (FallingLeaves_System.IsValid())
    {
        FallingLeaves_System.LoadSynchronous();
    }
    
    if (DenseFog_System.IsValid())
    {
        DenseFog_System.LoadSynchronous();
    }
    
    // Inicializar pools com componentes
    for (int32 i = 0; i < 20; i++)
    {
        UNiagaraComponent* PooledComponent = CreateDefaultSubobject<UNiagaraComponent>(*FString::Printf(TEXT("PooledVFX_%d"), i));
        PooledComponent->SetAutoDestroy(false);
        PooledComponent->SetVisibility(false);
        VFXPool_Environmental.Add(PooledComponent);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("VFX System Manager initialized with %d pooled components"), VFXPool_Environmental.Num());
}

void AVFXSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    CurrentFrameTime = DeltaTime;
    
    // Monitorar performance e ajustar qualidade se necessário
    if (CurrentFrameTime > 0.033f) // Abaixo de 30 FPS
    {
        AdjustVFXQualityForPerformance();
    }
    
    // Contar VFX ativos para debugging
    ActiveVFXCount = 0;
    for (UNiagaraComponent* Component : VFXPool_Environmental)
    {
        if (Component && Component->IsVisible())
        {
            ActiveVFXCount++;
        }
    }
}

UNiagaraComponent* AVFXSystemManager::SpawnVFXWithLOD(UNiagaraSystem* System, FVector Location, FRotator Rotation, float LifeTime)
{
    if (!System || !PlayerPawn)
    {
        return nullptr;
    }
    
    // Verificar se deve fazer culling
    if (ShouldCullVFX(Location))
    {
        return nullptr;
    }
    
    // Calcular LOD baseado na distância
    int32 LODLevel = CalculateLODLevel(Location);
    
    // Obter componente do pool
    UNiagaraComponent* VFXComponent = GetPooledVFXComponent(EVFXPoolType::Environmental);
    
    if (VFXComponent)
    {
        VFXComponent->SetAsset(System);
        VFXComponent->SetWorldLocationAndRotation(Location, Rotation);
        VFXComponent->SetVisibility(true);
        
        // Ajustar qualidade baseado no LOD
        switch (LODLevel)
        {
            case 0: // Close - máxima qualidade
                VFXComponent->SetFloatParameter(TEXT("ParticleCount"), 1.0f);
                VFXComponent->SetFloatParameter(TEXT("DetailLevel"), 1.0f);
                break;
                
            case 1: // Medium - qualidade reduzida
                VFXComponent->SetFloatParameter(TEXT("ParticleCount"), 0.6f);
                VFXComponent->SetFloatParameter(TEXT("DetailLevel"), 0.7f);
                break;
                
            case 2: // Far - qualidade mínima
                VFXComponent->SetFloatParameter(TEXT("ParticleCount"), 0.3f);
                VFXComponent->SetFloatParameter(TEXT("DetailLevel"), 0.4f);
                break;
        }
        
        VFXComponent->Activate();
        
        // Configurar auto-retorno ao pool
        if (LifeTime > 0.0f)
        {
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, VFXComponent]()
            {
                ReturnVFXToPool(VFXComponent, EVFXPoolType::Environmental);
            }, LifeTime, false);
        }
    }
    
    return VFXComponent;
}

UNiagaraComponent* AVFXSystemManager::GetPooledVFXComponent(EVFXPoolType PoolType)
{
    TArray<UNiagaraComponent*>* TargetPool = nullptr;
    
    switch (PoolType)
    {
        case EVFXPoolType::Environmental:
            TargetPool = &VFXPool_Environmental;
            break;
        case EVFXPoolType::Combat:
            TargetPool = &VFXPool_Combat;
            break;
        case EVFXPoolType::Atmospheric:
            TargetPool = &VFXPool_Atmospheric;
            break;
    }
    
    if (TargetPool)
    {
        for (UNiagaraComponent* Component : *TargetPool)
        {
            if (Component && !Component->IsVisible())
            {
                return Component;
            }
        }
    }
    
    return nullptr;
}

void AVFXSystemManager::ReturnVFXToPool(UNiagaraComponent* Component, EVFXPoolType PoolType)
{
    if (Component)
    {
        Component->Deactivate();
        Component->SetVisibility(false);
        Component->SetAsset(nullptr);
    }
}

void AVFXSystemManager::TriggerEnvironmentalTension(FVector Location, float Intensity)
{
    if (!PlayerPawn)
        return;
    
    float DistanceToPlayer = FVector::Dist(Location, PlayerPawn->GetActorLocation());
    
    // Só triggerar se estiver numa distância que crie tensão mas não seja óbvio
    if (DistanceToPlayer > 200.0f && DistanceToPlayer < 1000.0f)
    {
        // Folhas caindo sugerindo movimento
        if (FallingLeaves_System.IsValid())
        {
            SpawnVFXWithLOD(FallingLeaves_System.Get(), Location + FVector(0, 0, 200), FRotator::ZeroRotator, 3.0f);
        }
        
        // Galhos quebrando
        if (BreakingBranches_Debris.IsValid() && FMath::RandRange(0.0f, 1.0f) < Intensity)
        {
            FVector BranchLocation = Location + FVector(FMath::RandRange(-100, 100), FMath::RandRange(-100, 100), 50);
            SpawnVFXWithLOD(BreakingBranches_Debris.Get(), BranchLocation, FRotator::ZeroRotator, 2.0f);
        }
        
        // Pássaros fugindo ocasionalmente
        if (FlockPanic_Birds.IsValid() && FMath::RandRange(0.0f, 1.0f) < (Intensity * 0.3f))
        {
            FVector BirdLocation = Location + FVector(0, 0, 300);
            SpawnVFXWithLOD(FlockPanic_Birds.Get(), BirdLocation, FRotator::ZeroRotator, 5.0f);
        }
    }
}

void AVFXSystemManager::AdjustVFXQualityForPerformance()
{
    // Reduzir qualidade global dos VFX se performance estiver baixa
    static float QualityReductionFactor = 1.0f;
    
    if (CurrentFrameTime > 0.033f) // Abaixo de 30 FPS
    {
        QualityReductionFactor = FMath::Max(0.3f, QualityReductionFactor - 0.1f);
    }
    else if (CurrentFrameTime < 0.016f) // Acima de 60 FPS
    {
        QualityReductionFactor = FMath::Min(1.0f, QualityReductionFactor + 0.05f);
    }
    
    // Aplicar fator de qualidade a todos os VFX ativos
    for (UNiagaraComponent* Component : VFXPool_Environmental)
    {
        if (Component && Component->IsVisible())
        {
            Component->SetFloatParameter(TEXT("GlobalQuality"), QualityReductionFactor);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX Quality adjusted to: %f (Frame time: %f)"), QualityReductionFactor, CurrentFrameTime);
}

int32 AVFXSystemManager::CalculateLODLevel(FVector VFXLocation)
{
    if (!PlayerPawn)
        return 2; // Far LOD por defeito
    
    float Distance = FVector::Dist(VFXLocation, PlayerPawn->GetActorLocation());
    
    if (Distance <= LODDistance_Close)
        return 0; // Close LOD
    else if (Distance <= LODDistance_Medium)
        return 1; // Medium LOD
    else
        return 2; // Far LOD
}

bool AVFXSystemManager::ShouldCullVFX(FVector VFXLocation)
{
    if (!PlayerPawn)
        return true;
    
    // Culling baseado em distância máxima
    float Distance = FVector::Dist(VFXLocation, PlayerPawn->GetActorLocation());
    if (Distance > LODDistance_Far)
        return true;
    
    // Culling baseado em frustum (simplificado)
    // TODO: Implementar frustum culling adequado
    
    return false;
}