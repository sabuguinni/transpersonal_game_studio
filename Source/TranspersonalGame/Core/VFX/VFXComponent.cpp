#include "VFXComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UVFXComponent::UVFXComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    bAutoCleanup = true;
    MaxVFXDistance = 2000.0f;
    
    // Default auto VFX intervals
    AutoVFXIntervals.Add(EVFXType::DinosaurBreathing, 3.0f);
    AutoVFXIntervals.Add(EVFXType::Dust, 2.0f);
    AutoVFXIntervals.Add(EVFXType::Mist, 5.0f);
}

void UVFXComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Get VFX Manager
    if (UWorld* World = GetWorld())
    {
        VFXManager = World->GetSubsystem<UVFXSystemManager>();
        if (!VFXManager)
        {
            UE_LOG(LogTemp, Warning, TEXT("VFXComponent: Could not find VFXSystemManager"));
        }
    }
    
    // Initialize auto VFX timers
    for (const auto& Pair : AutoVFXIntervals)
    {
        AutoVFXTimers.Add(Pair.Key, 0.0f);
        AutoVFXEnabled.Add(Pair.Key, false);
    }
}

void UVFXComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DestroyAllVFX();
    Super::EndPlay(EndPlayReason);
}

void UVFXComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateAutoVFX(DeltaTime);
    
    if (bAutoCleanup)
    {
        CleanupInvalidVFX();
    }
}

UNiagaraComponent* UVFXComponent::SpawnVFX(EVFXType VFXType, FVector LocationOffset, FRotator RotationOffset)
{
    if (!VFXManager || !GetOwner())
    {
        return nullptr;
    }
    
    FVector SpawnLocation = GetOwner()->GetActorLocation() + LocationOffset;
    FRotator SpawnRotation = GetOwner()->GetActorRotation() + RotationOffset;
    
    if (!IsWithinMaxDistance(SpawnLocation))
    {
        return nullptr;
    }
    
    UNiagaraComponent* VFXComponent = VFXManager->SpawnVFXAttached(VFXType, GetOwner(), NAME_None, LocationOffset, RotationOffset);
    
    if (VFXComponent)
    {
        SpawnedVFX.Add(VFXComponent);
        UE_LOG(LogTemp, Log, TEXT("VFXComponent: Spawned VFX %d for actor %s"), (int32)VFXType, *GetOwner()->GetName());
    }
    
    return VFXComponent;
}

UNiagaraComponent* UVFXComponent::SpawnVFXAtSocket(EVFXType VFXType, FName SocketName, FVector LocationOffset, FRotator RotationOffset)
{
    if (!VFXManager || !GetOwner())
    {
        return nullptr;
    }
    
    FVector SocketLocation = GetSocketWorldLocation(SocketName);
    if (SocketLocation == FVector::ZeroVector)
    {
        // Fallback to actor location if socket not found
        return SpawnVFX(VFXType, LocationOffset, RotationOffset);
    }
    
    if (!IsWithinMaxDistance(SocketLocation))
    {
        return nullptr;
    }
    
    UNiagaraComponent* VFXComponent = VFXManager->SpawnVFXAttached(VFXType, GetOwner(), SocketName, LocationOffset, RotationOffset);
    
    if (VFXComponent)
    {
        SpawnedVFX.Add(VFXComponent);
        UE_LOG(LogTemp, Log, TEXT("VFXComponent: Spawned VFX %d at socket %s for actor %s"), (int32)VFXType, *SocketName.ToString(), *GetOwner()->GetName());
    }
    
    return VFXComponent;
}

UNiagaraComponent* UVFXComponent::SpawnVFXAtLocation(EVFXType VFXType, FVector WorldLocation, FRotator Rotation)
{
    if (!VFXManager)
    {
        return nullptr;
    }
    
    if (!IsWithinMaxDistance(WorldLocation))
    {
        return nullptr;
    }
    
    UNiagaraComponent* VFXComponent = VFXManager->SpawnVFXAtLocation(VFXType, WorldLocation, Rotation);
    
    if (VFXComponent)
    {
        SpawnedVFX.Add(VFXComponent);
        UE_LOG(LogTemp, Log, TEXT("VFXComponent: Spawned VFX %d at world location for actor %s"), (int32)VFXType, *GetOwner()->GetName());
    }
    
    return VFXComponent;
}

void UVFXComponent::DestroyAllVFX()
{
    if (!VFXManager)
    {
        return;
    }
    
    for (auto& VFXPtr : SpawnedVFX)
    {
        if (VFXPtr.IsValid())
        {
            VFXManager->DestroyVFX(VFXPtr.Get());
        }
    }
    
    SpawnedVFX.Empty();
    UE_LOG(LogTemp, Log, TEXT("VFXComponent: Destroyed all VFX for actor %s"), GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UVFXComponent::EnableAutoVFX(EVFXType VFXType, float Interval, bool bRandomInterval)
{
    AutoVFXIntervals.FindOrAdd(VFXType) = Interval;
    AutoVFXTimers.FindOrAdd(VFXType) = bRandomInterval ? FMath::RandRange(0.0f, Interval) : 0.0f;
    AutoVFXEnabled.FindOrAdd(VFXType) = true;
    
    UE_LOG(LogTemp, Log, TEXT("VFXComponent: Enabled auto VFX %d with interval %f"), (int32)VFXType, Interval);
}

void UVFXComponent::DisableAutoVFX(EVFXType VFXType)
{
    if (AutoVFXEnabled.Contains(VFXType))
    {
        AutoVFXEnabled[VFXType] = false;
        UE_LOG(LogTemp, Log, TEXT("VFXComponent: Disabled auto VFX %d"), (int32)VFXType);
    }
}

void UVFXComponent::DisableAllAutoVFX()
{
    for (auto& Pair : AutoVFXEnabled)
    {
        Pair.Value = false;
    }
    UE_LOG(LogTemp, Log, TEXT("VFXComponent: Disabled all auto VFX"));
}

void UVFXComponent::SetVFXParameter(UNiagaraComponent* VFXComponent, const FString& ParameterName, float Value)
{
    if (VFXManager)
    {
        VFXManager->SetVFXParameter(VFXComponent, ParameterName, Value);
    }
}

void UVFXComponent::SetVFXColorParameter(UNiagaraComponent* VFXComponent, const FString& ParameterName, FLinearColor Color)
{
    if (VFXManager)
    {
        VFXManager->SetVFXColorParameter(VFXComponent, ParameterName, Color);
    }
}

void UVFXComponent::SetVFXVectorParameter(UNiagaraComponent* VFXComponent, const FString& ParameterName, FVector Vector)
{
    if (VFXManager)
    {
        VFXManager->SetVFXVectorParameter(VFXComponent, ParameterName, Vector);
    }
}

void UVFXComponent::OnActorHit(AActor* HitActor, FVector HitLocation, FVector HitNormal)
{
    for (EVFXType VFXType : OnHitVFXTypes)
    {
        SpawnVFXAtLocation(VFXType, HitLocation, FRotationMatrix::MakeFromZ(HitNormal).Rotator());
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFXComponent: Actor hit, spawned %d hit VFX"), OnHitVFXTypes.Num());
}

void UVFXComponent::OnActorDestroyed()
{
    for (EVFXType VFXType : OnDestroyVFXTypes)
    {
        SpawnVFX(VFXType);
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFXComponent: Actor destroyed, spawned %d destroy VFX"), OnDestroyVFXTypes.Num());
}

void UVFXComponent::OnMovementStateChanged(bool bIsMoving, float MovementSpeed)
{
    if (bIsMoving)
    {
        for (EVFXType VFXType : OnMovementVFXTypes)
        {
            // Enable auto VFX for movement
            float Interval = FMath::Lerp(0.5f, 0.1f, MovementSpeed / 1000.0f); // Faster VFX for faster movement
            EnableAutoVFX(VFXType, Interval, true);
        }
    }
    else
    {
        for (EVFXType VFXType : OnMovementVFXTypes)
        {
            DisableAutoVFX(VFXType);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFXComponent: Movement state changed - Moving: %s, Speed: %f"), bIsMoving ? TEXT("true") : TEXT("false"), MovementSpeed);
}

void UVFXComponent::UpdateAutoVFX(float DeltaTime)
{
    for (auto& Pair : AutoVFXEnabled)
    {
        if (Pair.Value && AutoVFXIntervals.Contains(Pair.Key))
        {
            float& Timer = AutoVFXTimers.FindOrAdd(Pair.Key);
            Timer += DeltaTime;
            
            float Interval = AutoVFXIntervals[Pair.Key];
            if (Timer >= Interval)
            {
                SpawnVFX(Pair.Key);
                Timer = 0.0f;
            }
        }
    }
}

void UVFXComponent::CleanupInvalidVFX()
{
    SpawnedVFX.RemoveAll([](const TWeakObjectPtr<UNiagaraComponent>& VFXPtr)
    {
        return !VFXPtr.IsValid();
    });
}

FVector UVFXComponent::GetSocketWorldLocation(FName SocketName) const
{
    if (!GetOwner())
    {
        return FVector::ZeroVector;
    }
    
    // Try skeletal mesh component first
    if (USkeletalMeshComponent* SkeletalMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
    {
        if (SkeletalMesh->DoesSocketExist(SocketName))
        {
            return SkeletalMesh->GetSocketLocation(SocketName);
        }
    }
    
    // Try static mesh component
    if (UStaticMeshComponent* StaticMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>())
    {
        if (StaticMesh->DoesSocketExist(SocketName))
        {
            return StaticMesh->GetSocketLocation(SocketName);
        }
    }
    
    return FVector::ZeroVector;
}

FRotator UVFXComponent::GetSocketWorldRotation(FName SocketName) const
{
    if (!GetOwner())
    {
        return FRotator::ZeroRotator;
    }
    
    // Try skeletal mesh component first
    if (USkeletalMeshComponent* SkeletalMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
    {
        if (SkeletalMesh->DoesSocketExist(SocketName))
        {
            return SkeletalMesh->GetSocketRotation(SocketName);
        }
    }
    
    // Try static mesh component
    if (UStaticMeshComponent* StaticMesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>())
    {
        if (StaticMesh->DoesSocketExist(SocketName))
        {
            return StaticMesh->GetSocketRotation(SocketName);
        }
    }
    
    return FRotator::ZeroRotator;
}

bool UVFXComponent::IsWithinMaxDistance(const FVector& Location) const
{
    if (MaxVFXDistance <= 0.0f)
    {
        return true; // No distance limit
    }
    
    if (UWorld* World = GetWorld())
    {
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
        {
            float Distance = FVector::Dist(Location, PlayerPawn->GetActorLocation());
            return Distance <= MaxVFXDistance;
        }
    }
    
    return true; // Default to true if no player found
}