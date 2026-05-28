#include "Perf_LODManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UPerf_LODManager::UPerf_LODManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = UpdateInterval;
    
    // Default LOD settings optimized for prehistoric survival game
    LODSettings.HighQualityDistance = 2000.0f;    // 20m - Full detail for nearby objects
    LODSettings.MediumQualityDistance = 5000.0f;  // 50m - Reduced detail
    LODSettings.LowQualityDistance = 10000.0f;    // 100m - Minimal detail
    LODSettings.CullDistance = 20000.0f;          // 200m - Completely hidden
    LODSettings.bEnableDistanceCulling = true;
    LODSettings.bEnableTriangleReduction = true;
    LODSettings.TriangleReductionPercent = 0.5f;
    
    bEnableLODSystem = true;
    UpdateInterval = 0.1f;
    MaxActorsPerFrame = 50;
}

void UPerf_LODManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Perf_LODManager: System initialized with %d managed actors"), ManagedActors.Num());
    
    // Auto-register nearby actors for LOD management
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetRootComponent() && Actor->FindComponentByClass<UStaticMeshComponent>())
            {
                // Only register actors with static meshes (rocks, trees, structures)
                if (!Actor->IsA<APawn>()) // Don't manage pawns automatically
                {
                    RegisterActorForLOD(Actor);
                }
            }
        }
    }
}

void UPerf_LODManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableLODSystem)
        return;
        
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime >= UpdateInterval)
    {
        UpdateLODLevels();
        LastUpdateTime = CurrentTime;
    }
}

void UPerf_LODManager::UpdateLODLevels()
{
    if (ManagedActors.Num() == 0)
        return;
        
    APawn* PlayerPawn = GetPlayerPawn();
    if (!PlayerPawn)
        return;
        
    int32 ActorsProcessedThisFrame = 0;
    
    for (int32 i = 0; i < ManagedActors.Num() && ActorsProcessedThisFrame < MaxActorsPerFrame; i++)
    {
        FPerf_ActorLODData& ActorData = ManagedActors[i];
        
        if (!ActorData.Actor.IsValid())
        {
            // Remove invalid actors
            ManagedActors.RemoveAt(i);
            i--;
            continue;
        }
        
        // Calculate distance to player
        ActorData.DistanceToPlayer = CalculateDistanceToPlayer(ActorData.Actor.Get());
        
        // Update LOD based on distance
        UpdateActorLOD(ActorData);
        
        ActorsProcessedThisFrame++;
    }
    
    // Log performance stats periodically
    static float LastLogTime = 0.0f;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastLogTime > 5.0f) // Log every 5 seconds
    {
        int32 High, Medium, Low, Culled;
        GetLODDistribution(High, Medium, Low, Culled);
        UE_LOG(LogTemp, Log, TEXT("LOD Distribution - High: %d, Medium: %d, Low: %d, Culled: %d"), High, Medium, Low, Culled);
        LastLogTime = CurrentTime;
    }
}

void UPerf_LODManager::RegisterActorForLOD(AActor* Actor)
{
    if (!Actor)
        return;
        
    // Check if already registered
    for (const FPerf_ActorLODData& Data : ManagedActors)
    {
        if (Data.Actor == Actor)
            return;
    }
    
    FPerf_ActorLODData NewData;
    NewData.Actor = Actor;
    NewData.CurrentLOD = EPerf_LODLevel::LOD_High;
    NewData.bIsCulled = false;
    NewData.LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    // Calculate original triangle count
    if (UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>())
    {
        if (UStaticMesh* StaticMesh = MeshComp->GetStaticMesh())
        {
            NewData.OriginalTriangleCount = StaticMesh->GetNumTriangles(0);
        }
    }
    
    ManagedActors.Add(NewData);
    UE_LOG(LogTemp, Log, TEXT("Registered actor for LOD: %s"), *Actor->GetName());
}

void UPerf_LODManager::UnregisterActorFromLOD(AActor* Actor)
{
    if (!Actor)
        return;
        
    for (int32 i = 0; i < ManagedActors.Num(); i++)
    {
        if (ManagedActors[i].Actor == Actor)
        {
            ManagedActors.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("Unregistered actor from LOD: %s"), *Actor->GetName());
            return;
        }
    }
}

EPerf_LODLevel UPerf_LODManager::GetActorLODLevel(AActor* Actor) const
{
    for (const FPerf_ActorLODData& Data : ManagedActors)
    {
        if (Data.Actor == Actor)
            return Data.CurrentLOD;
    }
    return EPerf_LODLevel::LOD_High;
}

void UPerf_LODManager::SetLODSettings(const FPerf_LODSettings& NewSettings)
{
    LODSettings = NewSettings;
    ForceUpdateAllActors();
}

void UPerf_LODManager::ForceUpdateAllActors()
{
    for (FPerf_ActorLODData& ActorData : ManagedActors)
    {
        if (ActorData.Actor.IsValid())
        {
            ActorData.DistanceToPlayer = CalculateDistanceToPlayer(ActorData.Actor.Get());
            UpdateActorLOD(ActorData);
        }
    }
}

void UPerf_LODManager::UpdateActorLOD(FPerf_ActorLODData& ActorData)
{
    if (!ActorData.Actor.IsValid())
        return;
        
    EPerf_LODLevel NewLOD = EPerf_LODLevel::LOD_High;
    
    // Determine LOD level based on distance
    if (ActorData.DistanceToPlayer > LODSettings.CullDistance && LODSettings.bEnableDistanceCulling)
    {
        NewLOD = EPerf_LODLevel::LOD_Culled;
    }
    else if (ActorData.DistanceToPlayer > LODSettings.LowQualityDistance)
    {
        NewLOD = EPerf_LODLevel::LOD_Low;
    }
    else if (ActorData.DistanceToPlayer > LODSettings.MediumQualityDistance)
    {
        NewLOD = EPerf_LODLevel::LOD_Medium;
    }
    else
    {
        NewLOD = EPerf_LODLevel::LOD_High;
    }
    
    // Apply LOD if changed
    if (NewLOD != ActorData.CurrentLOD)
    {
        ActorData.CurrentLOD = NewLOD;
        ApplyLODLevel(ActorData.Actor.Get(), NewLOD);
        ActorData.LastUpdateTime = GetWorld()->GetTimeSeconds();
    }
}

void UPerf_LODManager::ApplyLODLevel(AActor* Actor, EPerf_LODLevel LODLevel)
{
    if (!Actor)
        return;
        
    UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (!MeshComp)
        return;
        
    switch (LODLevel)
    {
        case EPerf_LODLevel::LOD_High:
            SetMeshVisibility(MeshComp, true);
            OptimizeMeshLOD(MeshComp, LODLevel);
            break;
            
        case EPerf_LODLevel::LOD_Medium:
            SetMeshVisibility(MeshComp, true);
            OptimizeMeshLOD(MeshComp, LODLevel);
            break;
            
        case EPerf_LODLevel::LOD_Low:
            SetMeshVisibility(MeshComp, true);
            OptimizeMeshLOD(MeshComp, LODLevel);
            break;
            
        case EPerf_LODLevel::LOD_Culled:
            SetMeshVisibility(MeshComp, false);
            break;
    }
}

float UPerf_LODManager::CalculateDistanceToPlayer(AActor* Actor) const
{
    if (!Actor)
        return 0.0f;
        
    APawn* PlayerPawn = GetPlayerPawn();
    if (!PlayerPawn)
        return 0.0f;
        
    return FVector::Dist(Actor->GetActorLocation(), PlayerPawn->GetActorLocation());
}

APawn* UPerf_LODManager::GetPlayerPawn() const
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            return PC->GetPawn();
        }
    }
    return nullptr;
}

void UPerf_LODManager::OptimizeMeshLOD(UStaticMeshComponent* MeshComp, EPerf_LODLevel LODLevel)
{
    if (!MeshComp || !LODSettings.bEnableTriangleReduction)
        return;
        
    // Force specific LOD level based on distance
    int32 ForcedLOD = static_cast<int32>(LODLevel);
    MeshComp->SetForcedLodModel(ForcedLOD + 1); // UE5 LOD indices start at 1
}

void UPerf_LODManager::SetMeshVisibility(UStaticMeshComponent* MeshComp, bool bVisible)
{
    if (MeshComp)
    {
        MeshComp->SetVisibility(bVisible);
        MeshComp->SetCollisionEnabled(bVisible ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
    }
}

int32 UPerf_LODManager::GetCulledActorCount() const
{
    int32 Count = 0;
    for (const FPerf_ActorLODData& Data : ManagedActors)
    {
        if (Data.CurrentLOD == EPerf_LODLevel::LOD_Culled)
            Count++;
    }
    return Count;
}

float UPerf_LODManager::GetAverageDistanceToPlayer() const
{
    if (ManagedActors.Num() == 0)
        return 0.0f;
        
    float TotalDistance = 0.0f;
    for (const FPerf_ActorLODData& Data : ManagedActors)
    {
        TotalDistance += Data.DistanceToPlayer;
    }
    
    return TotalDistance / ManagedActors.Num();
}

void UPerf_LODManager::GetLODDistribution(int32& HighLOD, int32& MediumLOD, int32& LowLOD, int32& CulledLOD) const
{
    HighLOD = MediumLOD = LowLOD = CulledLOD = 0;
    
    for (const FPerf_ActorLODData& Data : ManagedActors)
    {
        switch (Data.CurrentLOD)
        {
            case EPerf_LODLevel::LOD_High:
                HighLOD++;
                break;
            case EPerf_LODLevel::LOD_Medium:
                MediumLOD++;
                break;
            case EPerf_LODLevel::LOD_Low:
                LowLOD++;
                break;
            case EPerf_LODLevel::LOD_Culled:
                CulledLOD++;
                break;
        }
    }
}