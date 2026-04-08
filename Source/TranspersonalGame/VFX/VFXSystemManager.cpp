#include "VFXSystemManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UVFXSystemManager::UVFXSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update LOD 10x por segundo
}

void UVFXSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    InitializeVFXRegistry();
    
    // Pre-populate pools para efeitos comuns
    CreatePooledVFX("BloodSplatter", 10);
    CreatePooledVFX("DirtKickup", 15);
    CreatePooledVFX("LeafRustle", 20);
    CreatePooledVFX("Sparks", 8);
    CreatePooledVFX("SmokePuff", 12);
}

void UVFXSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateVFXLOD();
    CleanupExpiredVFX();
}

UNiagaraComponent* UVFXSystemManager::SpawnVFX(const FString& EffectName, const FVector& Location, const FRotator& Rotation, AActor* AttachToActor)
{
    if (!VFXRegistry.Contains(EffectName))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX Effect '%s' não encontrado no registry"), *EffectName);
        return nullptr;
    }

    const FVFXDefinition& VFXDef = VFXRegistry[EffectName];
    
    // Verificar limites de performance
    if (ActiveVFX.Num() >= MaxActiveVFX)
    {
        UE_LOG(LogTemp, Warning, TEXT("Limite máximo de VFX atingido (%d)"), MaxActiveVFX);
        return nullptr;
    }

    // Verificar distância de culling
    if (PlayerPawn)
    {
        float DistanceToPlayer = GetDistanceToPlayer(Location);
        if (DistanceToPlayer > VFXDef.MaxViewDistance)
        {
            return nullptr; // Muito longe para renderizar
        }
    }

    // Tentar usar VFX do pool primeiro
    UNiagaraComponent* VFXComponent = GetPooledVFX(EffectName);
    
    if (!VFXComponent)
    {
        // Criar novo se pool vazio
        VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            VFXDef.NiagaraSystem.LoadSynchronous(),
            Location,
            Rotation
        );
    }
    else
    {
        // Configurar VFX do pool
        VFXComponent->SetWorldLocationAndRotation(Location, Rotation);
        VFXComponent->Activate();
    }

    if (VFXComponent)
    {
        // Attach se necessário
        if (AttachToActor)
        {
            VFXComponent->AttachToComponent(
                AttachToActor->GetRootComponent(),
                FAttachmentTransformRules::KeepWorldTransform
            );
        }

        // Aplicar LOD baseado na distância
        if (PlayerPawn)
        {
            float Distance = GetDistanceToPlayer(Location);
            int32 LODLevel = CalculateLODLevel(Distance);
            
            // Ajustar qualidade baseado no LOD
            switch (LODLevel)
            {
                case 0: // Próximo - qualidade máxima
                    VFXComponent->SetFloatParameter(TEXT("QualityScale"), 1.0f);
                    VFXComponent->SetFloatParameter(TEXT("ParticleCount"), 1.0f);
                    break;
                case 1: // Médio - qualidade reduzida
                    VFXComponent->SetFloatParameter(TEXT("QualityScale"), 0.7f);
                    VFXComponent->SetFloatParameter(TEXT("ParticleCount"), 0.6f);
                    break;
                case 2: // Longe - qualidade mínima
                    VFXComponent->SetFloatParameter(TEXT("QualityScale"), 0.4f);
                    VFXComponent->SetFloatParameter(TEXT("ParticleCount"), 0.3f);
                    break;
            }
        }

        ActiveVFX.Add(VFXComponent);

        // Auto-destroy se configurado
        if (VFXDef.bAutoDestroy && VFXDef.AutoDestroyDelay > 0)
        {
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(
                TimerHandle,
                [this, VFXComponent]()
                {
                    ReturnVFXToPool(VFXComponent);
                },
                VFXDef.AutoDestroyDelay,
                false
            );
        }
    }

    return VFXComponent;
}

void UVFXSystemManager::SpawnVFXAtLocation(EVFXCategory Category, const FString& EffectName, const FVector& Location, const FRotator& Rotation)
{
    SpawnVFX(EffectName, Location, Rotation);
}

void UVFXSystemManager::SpawnVFXAttached(EVFXCategory Category, const FString& EffectName, AActor* AttachToActor, const FName& SocketName)
{
    if (!AttachToActor) return;

    FVector Location = AttachToActor->GetActorLocation();
    FRotator Rotation = AttachToActor->GetActorRotation();

    // Se socket especificado, usar posição do socket
    if (SocketName != NAME_None)
    {
        USkeletalMeshComponent* SkelMesh = AttachToActor->FindComponentByClass<USkeletalMeshComponent>();
        if (SkelMesh && SkelMesh->DoesSocketExist(SocketName))
        {
            Location = SkelMesh->GetSocketLocation(SocketName);
            Rotation = SkelMesh->GetSocketRotation(SocketName);
        }
    }

    SpawnVFX(EffectName, Location, Rotation, AttachToActor);
}

void UVFXSystemManager::ReturnVFXToPool(UNiagaraComponent* VFXComponent)
{
    if (!VFXComponent) return;

    // Remover da lista de ativos
    ActiveVFX.Remove(VFXComponent);

    // Desativar e resetar
    VFXComponent->Deactivate();
    VFXComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

    // Encontrar o tipo de efeito e retornar ao pool apropriado
    for (auto& PoolPair : VFXPool)
    {
        if (PoolPair.Value.Num() < 10) // Limite do pool
        {
            PoolPair.Value.Add(VFXComponent);
            return;
        }
    }

    // Se pools cheios, destruir
    VFXComponent->DestroyComponent();
}

void UVFXSystemManager::UpdateVFXLOD()
{
    if (!PlayerPawn) return;

    for (UNiagaraComponent* VFX : ActiveVFX)
    {
        if (!VFX || !VFX->IsActive()) continue;

        float Distance = GetDistanceToPlayer(VFX->GetComponentLocation());
        int32 LODLevel = CalculateLODLevel(Distance);

        // Aplicar LOD dinâmico
        switch (LODLevel)
        {
            case 0: // Próximo
                VFX->SetFloatParameter(TEXT("LODScale"), 1.0f);
                break;
            case 1: // Médio
                VFX->SetFloatParameter(TEXT("LODScale"), 0.6f);
                break;
            case 2: // Longe
                VFX->SetFloatParameter(TEXT("LODScale"), 0.3f);
                break;
            case 3: // Muito longe - desativar
                VFX->Deactivate();
                break;
        }
    }
}

void UVFXSystemManager::SetGlobalVFXQuality(int32 QualityLevel)
{
    CurrentQualityLevel = FMath::Clamp(QualityLevel, 0, 3);

    // Aplicar configurações globais baseadas na qualidade
    float QualityMultiplier = 1.0f;
    switch (CurrentQualityLevel)
    {
        case 0: // Low
            QualityMultiplier = 0.3f;
            MaxActiveVFX = 50;
            VFXCullDistance = 5000.0f;
            break;
        case 1: // Medium
            QualityMultiplier = 0.6f;
            MaxActiveVFX = 75;
            VFXCullDistance = 7500.0f;
            break;
        case 2: // High
            QualityMultiplier = 0.8f;
            MaxActiveVFX = 100;
            VFXCullDistance = 10000.0f;
            break;
        case 3: // Ultra
            QualityMultiplier = 1.0f;
            MaxActiveVFX = 150;
            VFXCullDistance = 15000.0f;
            break;
    }

    // Aplicar qualidade a todos os VFX ativos
    for (UNiagaraComponent* VFX : ActiveVFX)
    {
        if (VFX && VFX->IsActive())
        {
            VFX->SetFloatParameter(TEXT("GlobalQuality"), QualityMultiplier);
        }
    }
}

void UVFXSystemManager::SpawnVFXBatch(const TArray<FString>& EffectNames, const TArray<FVector>& Locations)
{
    int32 MinCount = FMath::Min(EffectNames.Num(), Locations.Num());
    
    for (int32 i = 0; i < MinCount; i++)
    {
        SpawnVFX(EffectNames[i], Locations[i]);
    }
}

void UVFXSystemManager::InitializeVFXRegistry()
{
    // Registrar efeitos de ambiente
    VFXRegistry.Add("RainDrops", FVFXDefinition{
        "RainDrops", EVFXCategory::Environment, EVFXIntensity::Subtle,
        nullptr, 8000.0f, 20, true, 0.0f
    });

    VFXRegistry.Add("LeafRustle", FVFXDefinition{
        "LeafRustle", EVFXCategory::Environment, EVFXIntensity::Subtle,
        nullptr, 3000.0f, 15, true, 3.0f
    });

    // Registrar efeitos de combate
    VFXRegistry.Add("BloodSplatter", FVFXDefinition{
        "BloodSplatter", EVFXCategory::Combat, EVFXIntensity::Intense,
        nullptr, 5000.0f, 10, true, 8.0f
    });

    VFXRegistry.Add("DirtKickup", FVFXDefinition{
        "DirtKickup", EVFXCategory::Combat, EVFXIntensity::Moderate,
        nullptr, 4000.0f, 12, true, 4.0f
    });

    // Registrar efeitos de sobrevivência
    VFXRegistry.Add("CampfireSmoke", FVFXDefinition{
        "CampfireSmoke", EVFXCategory::Survival, EVFXIntensity::Moderate,
        nullptr, 6000.0f, 5, false, 0.0f
    });

    VFXRegistry.Add("Sparks", FVFXDefinition{
        "Sparks", EVFXCategory::Survival, EVFXIntensity::Moderate,
        nullptr, 2000.0f, 8, true, 2.0f
    });

    // Registrar efeitos místicos
    VFXRegistry.Add("GemGlow", FVFXDefinition{
        "GemGlow", EVFXCategory::Mystical, EVFXIntensity::Cinematic,
        nullptr, 10000.0f, 1, false, 0.0f
    });

    VFXRegistry.Add("TeleportEffect", FVFXDefinition{
        "TeleportEffect", EVFXCategory::Mystical, EVFXIntensity::Cinematic,
        nullptr, 8000.0f, 2, true, 5.0f
    });
}

void UVFXSystemManager::CleanupExpiredVFX()
{
    ActiveVFX.RemoveAll([](UNiagaraComponent* VFX)
    {
        return !VFX || !IsValid(VFX) || !VFX->IsActive();
    });
}

UNiagaraComponent* UVFXSystemManager::GetPooledVFX(const FString& EffectName)
{
    if (!VFXPool.Contains(EffectName) || VFXPool[EffectName].Num() == 0)
    {
        return nullptr;
    }

    UNiagaraComponent* VFX = VFXPool[EffectName].Pop();
    return IsValid(VFX) ? VFX : nullptr;
}

void UVFXSystemManager::CreatePooledVFX(const FString& EffectName, int32 PoolSize)
{
    if (!VFXRegistry.Contains(EffectName)) return;

    const FVFXDefinition& VFXDef = VFXRegistry[EffectName];
    if (!VFXDef.NiagaraSystem.LoadSynchronous()) return;

    TArray<UNiagaraComponent*>& Pool = VFXPool.FindOrAdd(EffectName);
    
    for (int32 i = 0; i < PoolSize; i++)
    {
        UNiagaraComponent* VFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            VFXDef.NiagaraSystem.LoadSynchronous(),
            FVector::ZeroVector,
            FRotator::ZeroRotator
        );
        
        if (VFX)
        {
            VFX->Deactivate();
            Pool.Add(VFX);
        }
    }
}

float UVFXSystemManager::GetDistanceToPlayer(const FVector& Location)
{
    if (!PlayerPawn) return 0.0f;
    return FVector::Dist(PlayerPawn->GetActorLocation(), Location);
}

int32 UVFXSystemManager::CalculateLODLevel(float Distance)
{
    if (Distance < 1000.0f) return 0;      // Próximo
    if (Distance < 3000.0f) return 1;      // Médio
    if (Distance < 6000.0f) return 2;      // Longe
    return 3;                              // Muito longe
}