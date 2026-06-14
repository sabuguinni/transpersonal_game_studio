#include "Perf_LODManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"

UPerf_LODManager::UPerf_LODManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS tick rate for LOD updates
    
    LODSettings = FPerf_LODSettings();
    UpdateFrequency = 0.1f;
    bEnableLODSystem = true;
    bDebugDrawLOD = false;
    LastUpdateTime = 0.0f;
    PlayerPawn = nullptr;
}

void UPerf_LODManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find player pawn
    PlayerPawn = GetPlayerPawn();
    
    // Initialize stats
    ResetLODStats();
    
    UE_LOG(LogTemp, Log, TEXT("Perf_LODManager: System initialized with %d managed actors"), ManagedActors.Num());
}

void UPerf_LODManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableLODSystem)
        return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update LODs at specified frequency
    if (CurrentTime - LastUpdateTime >= UpdateFrequency)
    {
        UpdateAllLODs();
        UpdateStats();
        LastUpdateTime = CurrentTime;
        
        if (bDebugDrawLOD)
        {
            DrawDebugLOD();
        }
    }
}

void UPerf_LODManager::RegisterActorForLOD(AActor* Actor, float CustomLODScale)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Perf_LODManager: Attempted to register null actor"));
        return;
    }
    
    ManagedActors.Add(Actor, CustomLODScale);
    ActorLODLevels.Add(Actor, EPerf_LODLevel::LOD0_Ultra);
    
    UE_LOG(LogTemp, Log, TEXT("Perf_LODManager: Registered actor %s with LOD scale %.2f"), 
           *Actor->GetName(), CustomLODScale);
}

void UPerf_LODManager::UnregisterActorFromLOD(AActor* Actor)
{
    if (!Actor)
        return;
    
    ManagedActors.Remove(Actor);
    ActorLODLevels.Remove(Actor);
    
    UE_LOG(LogTemp, Log, TEXT("Perf_LODManager: Unregistered actor %s"), *Actor->GetName());
}

EPerf_LODLevel UPerf_LODManager::GetActorLODLevel(AActor* Actor) const
{
    if (const EPerf_LODLevel* LODLevel = ActorLODLevels.Find(Actor))
    {
        return *LODLevel;
    }
    return EPerf_LODLevel::LOD4_Culled;
}

float UPerf_LODManager::GetDistanceToPlayer(AActor* Actor) const
{
    if (!Actor || !PlayerPawn)
        return 99999.0f;
    
    return FVector::Dist(Actor->GetActorLocation(), PlayerPawn->GetActorLocation());
}

void UPerf_LODManager::UpdateAllLODs()
{
    if (!PlayerPawn)
    {
        PlayerPawn = GetPlayerPawn();
        if (!PlayerPawn)
            return;
    }
    
    for (auto& ActorPair : ManagedActors)
    {
        AActor* Actor = ActorPair.Key;
        float LODScale = ActorPair.Value;
        
        if (IsValid(Actor))
        {
            UpdateActorLOD(Actor, LODScale);
        }
        else
        {
            // Clean up invalid actors
            ManagedActors.Remove(Actor);
            ActorLODLevels.Remove(Actor);
        }
    }
}

void UPerf_LODManager::SetLODSettings(const FPerf_LODSettings& NewSettings)
{
    LODSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("Perf_LODManager: LOD settings updated"));
}

FPerf_LODStats UPerf_LODManager::GetLODStats() const
{
    return CurrentStats;
}

void UPerf_LODManager::ResetLODStats()
{
    CurrentStats = FPerf_LODStats();
}

int32 UPerf_LODManager::GetTotalManagedActors() const
{
    return ManagedActors.Num();
}

void UPerf_LODManager::EnableDistanceCulling(bool bEnable)
{
    LODSettings.bEnableDistanceCulling = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Perf_LODManager: Distance culling %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_LODManager::EnableFrustumCulling(bool bEnable)
{
    LODSettings.bEnableFrustumCulling = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Perf_LODManager: Frustum culling %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

bool UPerf_LODManager::IsActorInViewFrustum(AActor* Actor) const
{
    if (!Actor || !PlayerPawn)
        return false;
    
    // Simple frustum check based on player view direction
    APlayerController* PC = Cast<APlayerController>(PlayerPawn->GetController());
    if (!PC)
        return true; // Default to visible if no player controller
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FVector ActorLocation = Actor->GetActorLocation();
    FVector PlayerForward = PlayerPawn->GetActorForwardVector();
    
    FVector ToActor = (ActorLocation - PlayerLocation).GetSafeNormal();
    float DotProduct = FVector::DotProduct(PlayerForward, ToActor);
    
    // Consider actor visible if within 120-degree cone
    return DotProduct > -0.5f;
}

void UPerf_LODManager::ToggleDebugDraw()
{
    bDebugDrawLOD = !bDebugDrawLOD;
    UE_LOG(LogTemp, Log, TEXT("Perf_LODManager: Debug draw %s"), 
           bDebugDrawLOD ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_LODManager::PrintLODStats()
{
    UE_LOG(LogTemp, Log, TEXT("=== LOD SYSTEM STATS ==="));
    UE_LOG(LogTemp, Log, TEXT("LOD0 (Ultra): %d actors"), CurrentStats.LOD0Count);
    UE_LOG(LogTemp, Log, TEXT("LOD1 (High): %d actors"), CurrentStats.LOD1Count);
    UE_LOG(LogTemp, Log, TEXT("LOD2 (Medium): %d actors"), CurrentStats.LOD2Count);
    UE_LOG(LogTemp, Log, TEXT("LOD3 (Low): %d actors"), CurrentStats.LOD3Count);
    UE_LOG(LogTemp, Log, TEXT("Culled: %d actors"), CurrentStats.CulledCount);
    UE_LOG(LogTemp, Log, TEXT("Total Managed: %d actors"), CurrentStats.TotalManagedObjects);
    UE_LOG(LogTemp, Log, TEXT("Average Distance: %.2f units"), CurrentStats.AverageDistance);
}

void UPerf_LODManager::UpdateActorLOD(AActor* Actor, float LODScale)
{
    if (!Actor)
        return;
    
    float Distance = GetDistanceToPlayer(Actor);
    EPerf_LODLevel NewLODLevel = CalculateLODLevel(Distance, LODScale);
    
    // Check frustum culling
    if (LODSettings.bEnableFrustumCulling && !IsActorInViewFrustum(Actor))
    {
        NewLODLevel = EPerf_LODLevel::LOD4_Culled;
    }
    
    // Update LOD level if changed
    EPerf_LODLevel* CurrentLOD = ActorLODLevels.Find(Actor);
    if (CurrentLOD && *CurrentLOD != NewLODLevel)
    {
        *CurrentLOD = NewLODLevel;
        ApplyLODToActor(Actor, NewLODLevel);
    }
}

EPerf_LODLevel UPerf_LODManager::CalculateLODLevel(float Distance, float LODScale) const
{
    if (!LODSettings.bEnableDistanceCulling)
        return EPerf_LODLevel::LOD0_Ultra;
    
    // Apply LOD scale multiplier
    float ScaledDistance = Distance / LODScale;
    
    if (ScaledDistance > LODSettings.CullDistance)
        return EPerf_LODLevel::LOD4_Culled;
    else if (ScaledDistance > LODSettings.LOD3Distance)
        return EPerf_LODLevel::LOD3_Low;
    else if (ScaledDistance > LODSettings.LOD2Distance)
        return EPerf_LODLevel::LOD2_Medium;
    else if (ScaledDistance > LODSettings.LOD1Distance)
        return EPerf_LODLevel::LOD1_High;
    else
        return EPerf_LODLevel::LOD0_Ultra;
}

void UPerf_LODManager::ApplyLODToActor(AActor* Actor, EPerf_LODLevel LODLevel)
{
    if (!Actor)
        return;
    
    // Handle visibility
    bool bShouldBeVisible = (LODLevel != EPerf_LODLevel::LOD4_Culled);
    Actor->SetActorHiddenInGame(!bShouldBeVisible);
    
    if (!bShouldBeVisible)
        return;
    
    // Apply LOD to static mesh components
    TArray<UStaticMeshComponent*> StaticMeshComponents;
    Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
    
    for (UStaticMeshComponent* MeshComp : StaticMeshComponents)
    {
        if (MeshComp)
        {
            // Force LOD level based on our system
            int32 ForcedLOD = static_cast<int32>(LODLevel);
            MeshComp->SetForcedLodModel(ForcedLOD + 1); // UE5 LOD is 1-based
        }
    }
    
    // Apply LOD to skeletal mesh components
    TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
    Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
    
    for (USkeletalMeshComponent* SkelMeshComp : SkeletalMeshComponents)
    {
        if (SkelMeshComp)
        {
            int32 ForcedLOD = static_cast<int32>(LODLevel);
            SkelMeshComp->SetForcedLOD(ForcedLOD + 1); // UE5 LOD is 1-based
        }
    }
}

void UPerf_LODManager::UpdateStats()
{
    // Reset counters
    CurrentStats.LOD0Count = 0;
    CurrentStats.LOD1Count = 0;
    CurrentStats.LOD2Count = 0;
    CurrentStats.LOD3Count = 0;
    CurrentStats.CulledCount = 0;
    CurrentStats.TotalManagedObjects = ManagedActors.Num();
    
    float TotalDistance = 0.0f;
    int32 ValidActors = 0;
    
    // Count LOD levels and calculate average distance
    for (const auto& ActorPair : ActorLODLevels)
    {
        AActor* Actor = ActorPair.Key;
        EPerf_LODLevel LODLevel = ActorPair.Value;
        
        if (IsValid(Actor))
        {
            switch (LODLevel)
            {
                case EPerf_LODLevel::LOD0_Ultra:
                    CurrentStats.LOD0Count++;
                    break;
                case EPerf_LODLevel::LOD1_High:
                    CurrentStats.LOD1Count++;
                    break;
                case EPerf_LODLevel::LOD2_Medium:
                    CurrentStats.LOD2Count++;
                    break;
                case EPerf_LODLevel::LOD3_Low:
                    CurrentStats.LOD3Count++;
                    break;
                case EPerf_LODLevel::LOD4_Culled:
                    CurrentStats.CulledCount++;
                    break;
            }
            
            TotalDistance += GetDistanceToPlayer(Actor);
            ValidActors++;
        }
    }
    
    CurrentStats.AverageDistance = ValidActors > 0 ? TotalDistance / ValidActors : 0.0f;
}

void UPerf_LODManager::DrawDebugLOD()
{
    if (!GetWorld())
        return;
    
    for (const auto& ActorPair : ActorLODLevels)
    {
        AActor* Actor = ActorPair.Key;
        EPerf_LODLevel LODLevel = ActorPair.Value;
        
        if (IsValid(Actor))
        {
            FVector ActorLocation = Actor->GetActorLocation();
            FColor DebugColor;
            
            switch (LODLevel)
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
            
            DrawDebugSphere(GetWorld(), ActorLocation, 50.0f, 8, DebugColor, false, UpdateFrequency + 0.1f);
        }
    }
}

APawn* UPerf_LODManager::GetPlayerPawn() const
{
    if (GetWorld())
    {
        return UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    }
    return nullptr;
}