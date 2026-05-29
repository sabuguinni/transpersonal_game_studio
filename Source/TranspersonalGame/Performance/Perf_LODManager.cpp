#include "Perf_LODManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "DrawDebugHelpers.h"
#include "Engine/Canvas.h"

UPerf_LODManager::UPerf_LODManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    UpdateFrequency = 0.1f;
    MaxActorsPerFrame = 50;
    bEnableLODSystem = true;
    bDebugLOD = false;
    CurrentUpdateIndex = 0;
    LastUpdateTime = 0.0f;
    TotalManagedActors = 0;
}

void UPerf_LODManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (bEnableLODSystem)
    {
        UE_LOG(LogTemp, Log, TEXT("Performance LOD Manager initialized"));
        
        // Auto-register nearby actors
        UWorld* World = GetWorld();
        if (World)
        {
            for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
            {
                AActor* Actor = *ActorItr;
                if (Actor && Actor != GetOwner())
                {
                    // Register actors with mesh components
                    if (Actor->FindComponentByClass<UStaticMeshComponent>() || 
                        Actor->FindComponentByClass<USkeletalMeshComponent>())
                    {
                        RegisterActor(Actor);
                    }
                }
            }
        }
    }
}

void UPerf_LODManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnableLODSystem)
    {
        UpdateLODSystem(DeltaTime);
        
        if (bDebugLOD)
        {
            DrawLODDebugInfo();
        }
    }
}

void UPerf_LODManager::RegisterActor(AActor* Actor)
{
    if (!Actor || !IsValid(Actor))
    {
        return;
    }
    
    // Check if already registered
    for (const FPerf_ActorLODData& ExistingData : ManagedActors)
    {
        if (ExistingData.Actor == Actor)
        {
            return; // Already registered
        }
    }
    
    FPerf_ActorLODData NewActorData;
    NewActorData.Actor = Actor;
    NewActorData.CurrentLOD = EPerf_LODLevel::LOD0_Ultra;
    NewActorData.LastDistance = 0.0f;
    NewActorData.LastUpdateTime = GetWorld()->GetTimeSeconds();
    NewActorData.bIsVisible = true;
    NewActorData.bPhysicsEnabled = true;
    
    ManagedActors.Add(NewActorData);
    TotalManagedActors = ManagedActors.Num();
    
    UE_LOG(LogTemp, Log, TEXT("LOD Manager: Registered actor %s"), *Actor->GetName());
}

void UPerf_LODManager::UnregisterActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    
    for (int32 i = ManagedActors.Num() - 1; i >= 0; --i)
    {
        if (ManagedActors[i].Actor == Actor)
        {
            ManagedActors.RemoveAt(i);
            TotalManagedActors = ManagedActors.Num();
            UE_LOG(LogTemp, Log, TEXT("LOD Manager: Unregistered actor %s"), *Actor->GetName());
            break;
        }
    }
}

void UPerf_LODManager::UpdateLODSystem(float DeltaTime)
{
    if (ManagedActors.Num() == 0)
    {
        return;
    }
    
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime < UpdateFrequency)
    {
        return;
    }
    
    LastUpdateTime = 0.0f;
    ProcessActorBatch();
    CleanupInvalidActors();
}

void UPerf_LODManager::ProcessActorBatch()
{
    int32 ActorsProcessed = 0;
    int32 StartIndex = CurrentUpdateIndex;
    
    FVector PlayerLocation = GetPlayerLocation();
    
    while (ActorsProcessed < MaxActorsPerFrame && CurrentUpdateIndex < ManagedActors.Num())
    {
        FPerf_ActorLODData& ActorData = ManagedActors[CurrentUpdateIndex];
        
        if (ActorData.Actor.IsValid())
        {
            float Distance = FVector::Dist(PlayerLocation, ActorData.Actor->GetActorLocation());
            UpdateActorLOD(ActorData, Distance);
            ActorsProcessed++;
        }
        
        CurrentUpdateIndex++;
    }
    
    // Reset index when we've processed all actors
    if (CurrentUpdateIndex >= ManagedActors.Num())
    {
        CurrentUpdateIndex = 0;
    }
}

void UPerf_LODManager::UpdateActorLOD(FPerf_ActorLODData& ActorData, float Distance)
{
    if (!ActorData.Actor.IsValid())
    {
        return;
    }
    
    EPerf_LODLevel NewLOD = CalculateLODLevel(Distance);
    ActorData.LastDistance = Distance;
    ActorData.LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    if (NewLOD != ActorData.CurrentLOD)
    {
        ActorData.CurrentLOD = NewLOD;
        ApplyLODToActor(ActorData.Actor.Get(), NewLOD);
        
        if (bDebugLOD)
        {
            UE_LOG(LogTemp, Log, TEXT("LOD Update: %s -> LOD%d (Distance: %.1f)"), 
                   *ActorData.Actor->GetName(), (int32)NewLOD, Distance);
        }
    }
}

EPerf_LODLevel UPerf_LODManager::CalculateLODLevel(float Distance) const
{
    if (Distance <= LODSettings.LOD0_Distance)
    {
        return EPerf_LODLevel::LOD0_Ultra;
    }
    else if (Distance <= LODSettings.LOD1_Distance)
    {
        return EPerf_LODLevel::LOD1_High;
    }
    else if (Distance <= LODSettings.LOD2_Distance)
    {
        return EPerf_LODLevel::LOD2_Medium;
    }
    else if (Distance <= LODSettings.LOD3_Distance)
    {
        return EPerf_LODLevel::LOD3_Low;
    }
    else if (Distance <= LODSettings.CullDistance)
    {
        return EPerf_LODLevel::LOD4_Culled;
    }
    
    return EPerf_LODLevel::LOD4_Culled;
}

void UPerf_LODManager::ApplyLODToActor(AActor* Actor, EPerf_LODLevel LODLevel)
{
    if (!Actor || !IsValid(Actor))
    {
        return;
    }
    
    // Handle visibility
    bool bShouldBeVisible = (LODLevel != EPerf_LODLevel::LOD4_Culled);
    Actor->SetActorHiddenInGame(!bShouldBeVisible);
    
    // Handle physics LOD
    if (LODSettings.bEnablePhysicsLOD)
    {
        bool bEnablePhysics = (LODLevel <= EPerf_LODLevel::LOD2_Medium);
        SetPhysicsLOD(Actor, bEnablePhysics);
    }
    
    // Handle mesh LOD
    UStaticMeshComponent* StaticMesh = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (StaticMesh)
    {
        SetMeshLOD(StaticMesh, LODLevel);
    }
    
    // Handle skeletal mesh LOD
    if (LODSettings.bEnableAnimationLOD)
    {
        USkeletalMeshComponent* SkelMesh = Actor->FindComponentByClass<USkeletalMeshComponent>();
        if (SkelMesh)
        {
            SetAnimationLOD(SkelMesh, LODLevel);
        }
    }
}

void UPerf_LODManager::SetPhysicsLOD(AActor* Actor, bool bEnabled)
{
    if (!Actor)
    {
        return;
    }
    
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp)
        {
            PrimComp->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
            PrimComp->SetSimulatePhysics(bEnabled);
        }
    }
}

void UPerf_LODManager::SetAnimationLOD(USkeletalMeshComponent* SkelMesh, EPerf_LODLevel LODLevel)
{
    if (!SkelMesh)
    {
        return;
    }
    
    // Force specific LOD level
    int32 LODIndex = FMath::Clamp((int32)LODLevel, 0, 3);
    SkelMesh->SetForcedLOD(LODIndex + 1); // UE5 LOD is 1-based
    
    // Disable animation for distant objects
    if (LODLevel >= EPerf_LODLevel::LOD3_Low)
    {
        SkelMesh->bPauseAnims = true;
    }
    else
    {
        SkelMesh->bPauseAnims = false;
    }
}

void UPerf_LODManager::SetMeshLOD(UStaticMeshComponent* StaticMesh, EPerf_LODLevel LODLevel)
{
    if (!StaticMesh)
    {
        return;
    }
    
    // Force specific LOD level
    int32 LODIndex = FMath::Clamp((int32)LODLevel, 0, 3);
    StaticMesh->SetForcedLodModel(LODIndex + 1); // UE5 LOD is 1-based
}

float UPerf_LODManager::GetPlayerDistance(AActor* Actor) const
{
    if (!Actor)
    {
        return 0.0f;
    }
    
    FVector PlayerLocation = GetPlayerLocation();
    return FVector::Dist(PlayerLocation, Actor->GetActorLocation());
}

bool UPerf_LODManager::IsActorInFrustum(AActor* Actor) const
{
    if (!Actor)
    {
        return false;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        return false;
    }
    
    // Simple frustum check - can be enhanced with proper frustum culling
    FVector PlayerLocation;
    FRotator PlayerRotation;
    PC->GetPlayerViewPoint(PlayerLocation, PlayerRotation);
    
    FVector ToActor = (Actor->GetActorLocation() - PlayerLocation).GetSafeNormal();
    FVector PlayerForward = PlayerRotation.Vector();
    
    float DotProduct = FVector::DotProduct(PlayerForward, ToActor);
    return DotProduct > 0.0f; // Actor is in front of player
}

void UPerf_LODManager::ForceUpdateAllActors()
{
    FVector PlayerLocation = GetPlayerLocation();
    
    for (FPerf_ActorLODData& ActorData : ManagedActors)
    {
        if (ActorData.Actor.IsValid())
        {
            float Distance = FVector::Dist(PlayerLocation, ActorData.Actor->GetActorLocation());
            UpdateActorLOD(ActorData, Distance);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("LOD Manager: Force updated %d actors"), ManagedActors.Num());
}

void UPerf_LODManager::SetLODSettings(const FPerf_LODSettings& NewSettings)
{
    LODSettings = NewSettings;
    ForceUpdateAllActors();
    UE_LOG(LogTemp, Log, TEXT("LOD Manager: Updated LOD settings"));
}

void UPerf_LODManager::CleanupInvalidActors()
{
    for (int32 i = ManagedActors.Num() - 1; i >= 0; --i)
    {
        if (!ManagedActors[i].Actor.IsValid())
        {
            ManagedActors.RemoveAt(i);
        }
    }
    
    TotalManagedActors = ManagedActors.Num();
}

AActor* UPerf_LODManager::GetPlayerActor() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        return nullptr;
    }
    
    return PC->GetPawn();
}

FVector UPerf_LODManager::GetPlayerLocation() const
{
    AActor* PlayerActor = GetPlayerActor();
    if (PlayerActor)
    {
        return PlayerActor->GetActorLocation();
    }
    
    return FVector::ZeroVector;
}

void UPerf_LODManager::DebugPrintLODStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== LOD Manager Status ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Managed Actors: %d"), TotalManagedActors);
    
    int32 LOD0Count = GetActorsAtLODLevel(EPerf_LODLevel::LOD0_Ultra);
    int32 LOD1Count = GetActorsAtLODLevel(EPerf_LODLevel::LOD1_High);
    int32 LOD2Count = GetActorsAtLODLevel(EPerf_LODLevel::LOD2_Medium);
    int32 LOD3Count = GetActorsAtLODLevel(EPerf_LODLevel::LOD3_Low);
    int32 LOD4Count = GetActorsAtLODLevel(EPerf_LODLevel::LOD4_Culled);
    
    UE_LOG(LogTemp, Warning, TEXT("LOD0 (Ultra): %d"), LOD0Count);
    UE_LOG(LogTemp, Warning, TEXT("LOD1 (High): %d"), LOD1Count);
    UE_LOG(LogTemp, Warning, TEXT("LOD2 (Medium): %d"), LOD2Count);
    UE_LOG(LogTemp, Warning, TEXT("LOD3 (Low): %d"), LOD3Count);
    UE_LOG(LogTemp, Warning, TEXT("LOD4 (Culled): %d"), LOD4Count);
}

void UPerf_LODManager::DrawLODDebugInfo()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    FVector PlayerLocation = GetPlayerLocation();
    
    for (const FPerf_ActorLODData& ActorData : ManagedActors)
    {
        if (ActorData.Actor.IsValid())
        {
            FColor DebugColor = FColor::White;
            
            switch (ActorData.CurrentLOD)
            {
                case EPerf_LODLevel::LOD0_Ultra:
                    DebugColor = FColor::Green;
                    break;
                case EPerf_LODLevel::LOD1_High:
                    DebugColor = FColor::Yellow;
                    break;
                case EPerf_LODLevel::LOD2_Medium:
                    DebugColor = FColor::Orange;
                    break;
                case EPerf_LODLevel::LOD3_Low:
                    DebugColor = FColor::Red;
                    break;
                case EPerf_LODLevel::LOD4_Culled:
                    DebugColor = FColor::Black;
                    break;
            }
            
            FVector ActorLocation = ActorData.Actor->GetActorLocation();
            DrawDebugSphere(World, ActorLocation, 50.0f, 8, DebugColor, false, 0.1f);
            
            FString DebugText = FString::Printf(TEXT("LOD%d\n%.0fm"), 
                                               (int32)ActorData.CurrentLOD, 
                                               ActorData.LastDistance);
            
            DrawDebugString(World, ActorLocation + FVector(0, 0, 100), DebugText, nullptr, DebugColor, 0.1f);
        }
    }
}

int32 UPerf_LODManager::GetActorsAtLODLevel(EPerf_LODLevel LODLevel) const
{
    int32 Count = 0;
    
    for (const FPerf_ActorLODData& ActorData : ManagedActors)
    {
        if (ActorData.CurrentLOD == LODLevel)
        {
            Count++;
        }
    }
    
    return Count;
}