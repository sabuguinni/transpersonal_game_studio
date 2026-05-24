#include "Perf_LODManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UPerf_LODManager::UPerf_LODManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    LODSettings = FPerf_LODSettings();
    UpdateFrequency = 0.1f;
    bDebugLOD = false;
    MaxActorsPerFrame = 50;
    LastUpdateTime = 0.0f;
    CurrentUpdateIndex = 0;
}

void UPerf_LODManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-register nearby static mesh actors
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AStaticMeshActor* MeshActor = *ActorItr;
            if (MeshActor && IsValid(MeshActor))
            {
                RegisterActorForLOD(MeshActor);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("LOD Manager initialized with %d managed actors"), ManagedActors.Num());
}

void UPerf_LODManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!LODSettings.bEnableAutomaticLOD)
        return;
        
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= UpdateFrequency)
    {
        UpdateLODForActorBatch();
        LastUpdateTime = 0.0f;
    }
}

void UPerf_LODManager::UpdateLODForAllActors()
{
    FVector ViewerLocation = GetViewerLocation();
    
    for (auto& ActorPtr : ManagedActors)
    {
        if (ActorPtr.IsValid())
        {
            AActor* Actor = ActorPtr.Get();
            EPerf_LODLevel NewLOD = CalculateLODLevel(Actor, ViewerLocation);
            SetLODForActor(Actor, NewLOD);
        }
    }
}

void UPerf_LODManager::UpdateLODForActorBatch()
{
    if (ManagedActors.Num() == 0)
        return;
        
    FVector ViewerLocation = GetViewerLocation();
    int32 ActorsToUpdate = FMath::Min(MaxActorsPerFrame, ManagedActors.Num());
    
    for (int32 i = 0; i < ActorsToUpdate; ++i)
    {
        int32 ActorIndex = (CurrentUpdateIndex + i) % ManagedActors.Num();
        
        if (ManagedActors[ActorIndex].IsValid())
        {
            AActor* Actor = ManagedActors[ActorIndex].Get();
            EPerf_LODLevel NewLOD = CalculateLODLevel(Actor, ViewerLocation);
            SetLODForActor(Actor, NewLOD);
        }
        else
        {
            // Remove invalid actors
            ManagedActors.RemoveAtSwap(ActorIndex);
        }
    }
    
    CurrentUpdateIndex = (CurrentUpdateIndex + ActorsToUpdate) % FMath::Max(1, ManagedActors.Num());
}

void UPerf_LODManager::SetLODForActor(AActor* Actor, EPerf_LODLevel LODLevel)
{
    if (!Actor || !IsValid(Actor))
        return;
        
    // Store current LOD level
    CurrentLODLevels.Add(Actor, LODLevel);
    
    // Apply LOD to static mesh components
    TArray<UStaticMeshComponent*> MeshComponents;
    Actor->GetComponents<UStaticMeshComponent>(MeshComponents);
    
    for (UStaticMeshComponent* MeshComp : MeshComponents)
    {
        if (MeshComp && IsValid(MeshComp))
        {
            ApplyLODToStaticMesh(MeshComp, LODLevel);
        }
    }
    
    // Handle culling
    if (LODLevel == EPerf_LODLevel::LOD_Culled)
    {
        Actor->SetActorHiddenInGame(true);
        Actor->SetActorTickEnabled(false);
    }
    else
    {
        Actor->SetActorHiddenInGame(false);
        Actor->SetActorTickEnabled(true);
    }
    
    // Debug visualization
    if (bDebugLOD)
    {
        FColor DebugColor = FColor::Green;
        switch (LODLevel)
        {
            case EPerf_LODLevel::LOD0_HighDetail: DebugColor = FColor::Green; break;
            case EPerf_LODLevel::LOD1_MediumDetail: DebugColor = FColor::Yellow; break;
            case EPerf_LODLevel::LOD2_LowDetail: DebugColor = FColor::Orange; break;
            case EPerf_LODLevel::LOD3_VeryLowDetail: DebugColor = FColor::Red; break;
            case EPerf_LODLevel::LOD_Culled: DebugColor = FColor::Black; break;
        }
        
        DrawDebugSphere(GetWorld(), Actor->GetActorLocation(), 50.0f, 8, DebugColor, false, UpdateFrequency + 0.1f);
    }
}

EPerf_LODLevel UPerf_LODManager::CalculateLODLevel(AActor* Actor, const FVector& ViewerLocation) const
{
    if (!Actor || !IsValid(Actor))
        return EPerf_LODLevel::LOD_Culled;
        
    float Distance = GetDistanceToViewer(Actor);
    
    if (Distance > LODSettings.CullDistance)
        return EPerf_LODLevel::LOD_Culled;
    else if (Distance > LODSettings.LOD3Distance)
        return EPerf_LODLevel::LOD3_VeryLowDetail;
    else if (Distance > LODSettings.LOD2Distance)
        return EPerf_LODLevel::LOD2_LowDetail;
    else if (Distance > LODSettings.LOD1Distance)
        return EPerf_LODLevel::LOD1_MediumDetail;
    else
        return EPerf_LODLevel::LOD0_HighDetail;
}

float UPerf_LODManager::GetDistanceToViewer(AActor* Actor) const
{
    if (!Actor || !IsValid(Actor))
        return 999999.0f;
        
    FVector ViewerLocation = GetViewerLocation();
    return FVector::Dist(Actor->GetActorLocation(), ViewerLocation);
}

void UPerf_LODManager::SetLODSettings(const FPerf_LODSettings& NewSettings)
{
    LODSettings = NewSettings;
    
    // Force update all actors with new settings
    if (LODSettings.bEnableAutomaticLOD)
    {
        UpdateLODForAllActors();
    }
}

void UPerf_LODManager::EnableLODDebugging(bool bEnable)
{
    bDebugLOD = bEnable;
}

int32 UPerf_LODManager::GetCulledActorCount() const
{
    int32 CulledCount = 0;
    for (const auto& LODPair : CurrentLODLevels)
    {
        if (LODPair.Value == EPerf_LODLevel::LOD_Culled)
        {
            CulledCount++;
        }
    }
    return CulledCount;
}

void UPerf_LODManager::RegisterActorForLOD(AActor* Actor)
{
    if (Actor && IsValid(Actor))
    {
        ManagedActors.AddUnique(Actor);
    }
}

void UPerf_LODManager::UnregisterActorFromLOD(AActor* Actor)
{
    if (Actor)
    {
        ManagedActors.RemoveAll([Actor](const TWeakObjectPtr<AActor>& WeakPtr)
        {
            return WeakPtr.Get() == Actor;
        });
        CurrentLODLevels.Remove(Actor);
    }
}

void UPerf_LODManager::OptimizeForFrameRate(float TargetFrameRate)
{
    float CurrentFrameRate = 1.0f / GetWorld()->GetDeltaSeconds();
    
    if (CurrentFrameRate < TargetFrameRate)
    {
        // Reduce LOD distances to improve performance
        LODSettings.LOD0Distance *= 0.8f;
        LODSettings.LOD1Distance *= 0.8f;
        LODSettings.LOD2Distance *= 0.8f;
        LODSettings.LOD3Distance *= 0.8f;
        LODSettings.CullDistance *= 0.9f;
        
        UE_LOG(LogTemp, Warning, TEXT("Performance optimization: Reduced LOD distances for target FPS %f"), TargetFrameRate);
        
        // Apply new settings immediately
        UpdateLODForAllActors();
    }
}

void UPerf_LODManager::ApplyLODToStaticMesh(UStaticMeshComponent* MeshComp, EPerf_LODLevel LODLevel)
{
    if (!MeshComp || !IsValid(MeshComp))
        return;
        
    // Force specific LOD level
    int32 LODIndex = static_cast<int32>(LODLevel);
    if (LODIndex < 4) // Only apply if not culled
    {
        MeshComp->SetForcedLodModel(LODIndex + 1); // UE5 LOD is 1-based
    }
    
    // Adjust shadow casting based on LOD
    switch (LODLevel)
    {
        case EPerf_LODLevel::LOD0_HighDetail:
        case EPerf_LODLevel::LOD1_MediumDetail:
            MeshComp->SetCastShadow(true);
            break;
        case EPerf_LODLevel::LOD2_LowDetail:
        case EPerf_LODLevel::LOD3_VeryLowDetail:
            MeshComp->SetCastShadow(false);
            break;
        case EPerf_LODLevel::LOD_Culled:
            MeshComp->SetVisibility(false);
            break;
    }
}

FVector UPerf_LODManager::GetViewerLocation() const
{
    UWorld* World = GetWorld();
    if (!World)
        return FVector::ZeroVector;
        
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (PC && PC->GetPawn())
    {
        return PC->GetPawn()->GetActorLocation();
    }
    
    return FVector::ZeroVector;
}