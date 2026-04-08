#include "VFXSystemCore.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UVFXSystemCore::UVFXSystemCore()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = VFXUpdateRate;
}

void UVFXSystemCore::BeginPlay()
{
    Super::BeginPlay();
    
    // Encontra referência ao jogador
    PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    
    // Inicializa sistema de performance baseado na plataforma
    SetVFXQualityLevel(2); // Médio por defeito
}

void UVFXSystemCore::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LODUpdateTimer += DeltaTime;
    
    if (LODUpdateTimer >= VFXUpdateRate)
    {
        UpdateVFXLOD();
        CleanupFinishedVFX();
        CullDistantVFX();
        LODUpdateTimer = 0.0f;
    }
}

UNiagaraComponent* UVFXSystemCore::SpawnVFXAtLocation(UNiagaraSystem* System, FVector Location, FRotator Rotation, FVFXSettings Settings)
{
    if (!System)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFXSystemCore: Tentativa de spawn VFX com sistema nulo"));
        return nullptr;
    }

    // Verifica limites de performance
    if (ActiveVFXComponents.Num() >= MaxActiveVFX && Settings.bCanBeCulled)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFXSystemCore: Limite máximo de VFX atingido, ignorando spawn"));
        return nullptr;
    }

    // Verifica distância para culling
    float DistanceToPlayer = CalculateDistanceToPlayer(Location);
    if (DistanceToPlayer > Settings.CullDistance)
    {
        return nullptr;
    }

    // Spawn do efeito
    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        System,
        Location,
        Rotation
    );

    if (VFXComponent)
    {
        ActiveVFXComponents.Add(VFXComponent);
        VFXSettingsMap.Add(VFXComponent, Settings);

        // Aplica LOD inicial baseado na distância
        int32 LODLevel = DetermineVFXLODLevel(DistanceToPlayer, Settings.Priority);
        VFXComponent->SetIntParameter(TEXT("LOD_Level"), LODLevel);

        UE_LOG(LogTemp, Log, TEXT("VFXSystemCore: VFX spawned at distance %.2f with LOD %d"), DistanceToPlayer, LODLevel);
    }

    return VFXComponent;
}

UNiagaraComponent* UVFXSystemCore::SpawnVFXAttached(UNiagaraSystem* System, USceneComponent* AttachToComponent, FName AttachPointName, FVFXSettings Settings)
{
    if (!System || !AttachToComponent)
    {
        return nullptr;
    }

    UNiagaraComponent* VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
        System,
        AttachToComponent,
        AttachPointName,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::KeepRelativeOffset,
        true
    );

    if (VFXComponent)
    {
        ActiveVFXComponents.Add(VFXComponent);
        VFXSettingsMap.Add(VFXComponent, Settings);
    }

    return VFXComponent;
}

void UVFXSystemCore::UpdateVFXLOD()
{
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    for (int32 i = ActiveVFXComponents.Num() - 1; i >= 0; --i)
    {
        UNiagaraComponent* VFXComp = ActiveVFXComponents[i];
        if (!IsValid(VFXComp))
        {
            ActiveVFXComponents.RemoveAt(i);
            continue;
        }

        FVFXSettings* Settings = VFXSettingsMap.Find(VFXComp);
        if (!Settings)
        {
            continue;
        }

        float Distance = FVector::Dist(PlayerLocation, VFXComp->GetComponentLocation());
        int32 NewLODLevel = DetermineVFXLODLevel(Distance, Settings->Priority);
        
        VFXComp->SetIntParameter(TEXT("LOD_Level"), NewLODLevel);
    }
}

void UVFXSystemCore::CullDistantVFX()
{
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    for (int32 i = ActiveVFXComponents.Num() - 1; i >= 0; --i)
    {
        UNiagaraComponent* VFXComp = ActiveVFXComponents[i];
        if (!IsValid(VFXComp))
        {
            ActiveVFXComponents.RemoveAt(i);
            continue;
        }

        FVFXSettings* Settings = VFXSettingsMap.Find(VFXComp);
        if (!Settings || !Settings->bCanBeCulled)
        {
            continue;
        }

        float Distance = FVector::Dist(PlayerLocation, VFXComp->GetComponentLocation());
        if (Distance > Settings->CullDistance)
        {
            VFXComp->DestroyComponent();
            ActiveVFXComponents.RemoveAt(i);
            VFXSettingsMap.Remove(VFXComp);
        }
    }
}

void UVFXSystemCore::SetVFXQualityLevel(int32 QualityLevel)
{
    switch (QualityLevel)
    {
        case 0: // Baixo
            MaxActiveVFX = 20;
            VFXUpdateRate = 0.2f; // 5 updates por segundo
            break;
        case 1: // Médio
            MaxActiveVFX = 35;
            VFXUpdateRate = 0.15f;
            break;
        case 2: // Alto
            MaxActiveVFX = 50;
            VFXUpdateRate = 0.1f;
            break;
        case 3: // Ultra
            MaxActiveVFX = 75;
            VFXUpdateRate = 0.05f; // 20 updates por segundo
            break;
    }
    
    // Actualiza tick interval
    PrimaryComponentTick.TickInterval = VFXUpdateRate;
}

void UVFXSystemCore::CleanupFinishedVFX()
{
    for (int32 i = ActiveVFXComponents.Num() - 1; i >= 0; --i)
    {
        UNiagaraComponent* VFXComp = ActiveVFXComponents[i];
        if (!IsValid(VFXComp) || !VFXComp->IsActive())
        {
            if (IsValid(VFXComp))
            {
                VFXSettingsMap.Remove(VFXComp);
            }
            ActiveVFXComponents.RemoveAt(i);
        }
    }
}

float UVFXSystemCore::CalculateDistanceToPlayer(FVector Location) const
{
    if (!PlayerPawn)
    {
        return 0.0f;
    }
    
    return FVector::Dist(PlayerPawn->GetActorLocation(), Location);
}

int32 UVFXSystemCore::DetermineVFXLODLevel(float Distance, EVFXPriority Priority) const
{
    // LOD Chain a 3 níveis baseado em distância e prioridade
    float LOD0Distance = 1000.0f; // Máxima qualidade
    float LOD1Distance = 3000.0f; // Qualidade média
    float LOD2Distance = 6000.0f; // Qualidade baixa
    
    // Ajusta distâncias baseado na prioridade
    switch (Priority)
    {
        case EVFXPriority::Critical:
            LOD0Distance *= 2.0f;
            LOD1Distance *= 2.0f;
            LOD2Distance *= 2.0f;
            break;
        case EVFXPriority::High:
            LOD0Distance *= 1.5f;
            LOD1Distance *= 1.5f;
            LOD2Distance *= 1.5f;
            break;
        case EVFXPriority::Low:
        case EVFXPriority::Culled:
            LOD0Distance *= 0.5f;
            LOD1Distance *= 0.5f;
            LOD2Distance *= 0.5f;
            break;
    }
    
    if (Distance <= LOD0Distance)
    {
        return 0; // Máxima qualidade
    }
    else if (Distance <= LOD1Distance)
    {
        return 1; // Qualidade média
    }
    else if (Distance <= LOD2Distance)
    {
        return 2; // Qualidade baixa
    }
    else
    {
        return 3; // Mínima qualidade ou culled
    }
}