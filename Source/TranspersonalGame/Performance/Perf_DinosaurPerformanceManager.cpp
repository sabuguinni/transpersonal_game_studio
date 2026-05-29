#include "Perf_DinosaurPerformanceManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"

UPerf_DinosaurPerformanceManager::UPerf_DinosaurPerformanceManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    LODSettings = FPerf_DinosaurLODSettings();
    UpdateFrequency = 0.1f;
    bEnableDynamicLOD = true;
    bEnablePerformanceLogging = false;
    
    TimeSinceLastUpdate = 0.0f;
    FrameTimeAccumulator = 0.0f;
    FrameCount = 0;
}

void UPerf_DinosaurPerformanceManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize performance tracking
    CurrentStats = FPerf_DinosaurPerformanceStats();
    
    // Find all existing dinosaur pawns in the world
    UWorld* World = GetWorld();
    if (World)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            APawn* Pawn = Cast<APawn>(Actor);
            if (Pawn && IsDinosaurPawn(Pawn))
            {
                RegisterDinosaur(Pawn);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurPerformanceManager initialized with %d dinosaurs"), RegisteredDinosaurs.Num());
}

void UPerf_DinosaurPerformanceManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableDynamicLOD)
    {
        return;
    }
    
    TimeSinceLastUpdate += DeltaTime;
    FrameTimeAccumulator += DeltaTime;
    FrameCount++;
    
    if (TimeSinceLastUpdate >= UpdateFrequency)
    {
        CleanupInvalidReferences();
        UpdateDinosaurLOD();
        UpdatePerformanceStats();
        
        if (bEnablePerformanceLogging)
        {
            LogPerformanceData();
        }
        
        TimeSinceLastUpdate = 0.0f;
    }
}

void UPerf_DinosaurPerformanceManager::UpdateDinosaurLOD()
{
    if (RegisteredDinosaurs.Num() == 0)
    {
        return;
    }
    
    // Sort dinosaurs by distance to player
    SortDinosaursByDistance();
    
    // Apply visibility limits
    ApplyVisibilityLimits();
    
    // Update LOD for each dinosaur
    for (const TWeakObjectPtr<APawn>& DinosaurPtr : RegisteredDinosaurs)
    {
        if (APawn* Dinosaur = DinosaurPtr.Get())
        {
            float Distance = CalculateDistanceToPlayer(Dinosaur);
            EPerf_DinosaurLODLevel NewLODLevel = DetermineLODLevel(Distance);
            
            EPerf_DinosaurLODLevel* CurrentLODPtr = DinosaurLODLevels.Find(DinosaurPtr);
            EPerf_DinosaurLODLevel CurrentLOD = CurrentLODPtr ? *CurrentLODPtr : EPerf_DinosaurLODLevel::HighDetail;
            
            if (CurrentLOD != NewLODLevel)
            {
                SetDinosaurLODLevel(Dinosaur, NewLODLevel);
            }
        }
    }
}

void UPerf_DinosaurPerformanceManager::RegisterDinosaur(APawn* DinosaurPawn)
{
    if (!DinosaurPawn || !IsDinosaurPawn(DinosaurPawn))
    {
        return;
    }
    
    TWeakObjectPtr<APawn> DinosaurPtr(DinosaurPawn);
    if (!RegisteredDinosaurs.Contains(DinosaurPtr))
    {
        RegisteredDinosaurs.Add(DinosaurPtr);
        DinosaurLODLevels.Add(DinosaurPtr, EPerf_DinosaurLODLevel::HighDetail);
        
        UE_LOG(LogTemp, Log, TEXT("Registered dinosaur: %s"), *DinosaurPawn->GetName());
    }
}

void UPerf_DinosaurPerformanceManager::UnregisterDinosaur(APawn* DinosaurPawn)
{
    if (!DinosaurPawn)
    {
        return;
    }
    
    TWeakObjectPtr<APawn> DinosaurPtr(DinosaurPawn);
    RegisteredDinosaurs.Remove(DinosaurPtr);
    DinosaurLODLevels.Remove(DinosaurPtr);
    
    UE_LOG(LogTemp, Log, TEXT("Unregistered dinosaur: %s"), *DinosaurPawn->GetName());
}

EPerf_DinosaurLODLevel UPerf_DinosaurPerformanceManager::GetDinosaurLODLevel(APawn* DinosaurPawn) const
{
    if (!DinosaurPawn)
    {
        return EPerf_DinosaurLODLevel::Culled;
    }
    
    TWeakObjectPtr<APawn> DinosaurPtr(DinosaurPawn);
    const EPerf_DinosaurLODLevel* LODLevel = DinosaurLODLevels.Find(DinosaurPtr);
    return LODLevel ? *LODLevel : EPerf_DinosaurLODLevel::HighDetail;
}

void UPerf_DinosaurPerformanceManager::SetDinosaurLODLevel(APawn* DinosaurPawn, EPerf_DinosaurLODLevel LODLevel)
{
    if (!DinosaurPawn)
    {
        return;
    }
    
    TWeakObjectPtr<APawn> DinosaurPtr(DinosaurPawn);
    DinosaurLODLevels.Add(DinosaurPtr, LODLevel);
    
    ApplyLODToDinosaur(DinosaurPawn, LODLevel);
}

void UPerf_DinosaurPerformanceManager::OptimizeDinosaurRendering()
{
    for (const TWeakObjectPtr<APawn>& DinosaurPtr : RegisteredDinosaurs)
    {
        if (APawn* Dinosaur = DinosaurPtr.Get())
        {
            EPerf_DinosaurLODLevel LODLevel = GetDinosaurLODLevel(Dinosaur);
            
            // Apply rendering optimizations based on LOD level
            if (USkeletalMeshComponent* SkeletalMesh = Dinosaur->FindComponentByClass<USkeletalMeshComponent>())
            {
                switch (LODLevel)
                {
                    case EPerf_DinosaurLODLevel::HighDetail:
                        SkeletalMesh->SetForcedLOD(0);
                        SkeletalMesh->SetVisibility(true);
                        break;
                        
                    case EPerf_DinosaurLODLevel::MediumDetail:
                        SkeletalMesh->SetForcedLOD(1);
                        SkeletalMesh->SetVisibility(true);
                        break;
                        
                    case EPerf_DinosaurLODLevel::LowDetail:
                        SkeletalMesh->SetForcedLOD(2);
                        SkeletalMesh->SetVisibility(true);
                        break;
                        
                    case EPerf_DinosaurLODLevel::Culled:
                        SkeletalMesh->SetVisibility(false);
                        break;
                }
            }
        }
    }
}

void UPerf_DinosaurPerformanceManager::CullDistantDinosaurs()
{
    for (const TWeakObjectPtr<APawn>& DinosaurPtr : RegisteredDinosaurs)
    {
        if (APawn* Dinosaur = DinosaurPtr.Get())
        {
            float Distance = CalculateDistanceToPlayer(Dinosaur);
            
            if (Distance > LODSettings.CullingDistance)
            {
                SetDinosaurLODLevel(Dinosaur, EPerf_DinosaurLODLevel::Culled);
            }
        }
    }
}

void UPerf_DinosaurPerformanceManager::SetMaxVisibleDinosaurs(int32 MaxCount)
{
    LODSettings.MaxVisibleDinosaurs = MaxCount;
    ApplyVisibilityLimits();
}

void UPerf_DinosaurPerformanceManager::EnableAnimationCulling(bool bEnable)
{
    LODSettings.bEnableAnimationCulling = bEnable;
    
    for (const TWeakObjectPtr<APawn>& DinosaurPtr : RegisteredDinosaurs)
    {
        if (APawn* Dinosaur = DinosaurPtr.Get())
        {
            if (USkeletalMeshComponent* SkeletalMesh = Dinosaur->FindComponentByClass<USkeletalMeshComponent>())
            {
                if (bEnable)
                {
                    float Distance = CalculateDistanceToPlayer(Dinosaur);
                    bool bShouldCullAnimation = Distance > LODSettings.MediumDetailDistance;
                    SkeletalMesh->bPauseAnims = bShouldCullAnimation;
                }
                else
                {
                    SkeletalMesh->bPauseAnims = false;
                }
            }
        }
    }
}

void UPerf_DinosaurPerformanceManager::EnablePhysicsCulling(bool bEnable)
{
    LODSettings.bEnablePhysicsCulling = bEnable;
    
    for (const TWeakObjectPtr<APawn>& DinosaurPtr : RegisteredDinosaurs)
    {
        if (APawn* Dinosaur = DinosaurPtr.Get())
        {
            float Distance = CalculateDistanceToPlayer(Dinosaur);
            bool bShouldCullPhysics = bEnable && (Distance > LODSettings.LowDetailDistance);
            
            Dinosaur->SetActorEnableCollision(!bShouldCullPhysics);
        }
    }
}

void UPerf_DinosaurPerformanceManager::UpdatePerformanceStats()
{
    CurrentStats.TotalDinosaurs = RegisteredDinosaurs.Num();
    CurrentStats.VisibleDinosaurs = 0;
    CurrentStats.HighDetailDinosaurs = 0;
    CurrentStats.MediumDetailDinosaurs = 0;
    CurrentStats.LowDetailDinosaurs = 0;
    CurrentStats.CulledDinosaurs = 0;
    
    for (const TWeakObjectPtr<APawn>& DinosaurPtr : RegisteredDinosaurs)
    {
        if (APawn* Dinosaur = DinosaurPtr.Get())
        {
            EPerf_DinosaurLODLevel LODLevel = GetDinosaurLODLevel(Dinosaur);
            
            switch (LODLevel)
            {
                case EPerf_DinosaurLODLevel::HighDetail:
                    CurrentStats.HighDetailDinosaurs++;
                    CurrentStats.VisibleDinosaurs++;
                    break;
                case EPerf_DinosaurLODLevel::MediumDetail:
                    CurrentStats.MediumDetailDinosaurs++;
                    CurrentStats.VisibleDinosaurs++;
                    break;
                case EPerf_DinosaurLODLevel::LowDetail:
                    CurrentStats.LowDetailDinosaurs++;
                    CurrentStats.VisibleDinosaurs++;
                    break;
                case EPerf_DinosaurLODLevel::Culled:
                    CurrentStats.CulledDinosaurs++;
                    break;
            }
        }
    }
    
    // Calculate average frame time
    if (FrameCount > 0)
    {
        CurrentStats.AverageFrameTime = FrameTimeAccumulator / FrameCount;
        FrameTimeAccumulator = 0.0f;
        FrameCount = 0;
    }
    
    // Estimate dinosaur render cost
    CurrentStats.DinosaurRenderCost = (CurrentStats.HighDetailDinosaurs * 1.0f) + 
                                     (CurrentStats.MediumDetailDinosaurs * 0.6f) + 
                                     (CurrentStats.LowDetailDinosaurs * 0.3f);
}

void UPerf_DinosaurPerformanceManager::ApplyLODToDinosaur(APawn* DinosaurPawn, EPerf_DinosaurLODLevel LODLevel)
{
    if (!DinosaurPawn)
    {
        return;
    }
    
    // Apply mesh LOD
    if (USkeletalMeshComponent* SkeletalMesh = DinosaurPawn->FindComponentByClass<USkeletalMeshComponent>())
    {
        switch (LODLevel)
        {
            case EPerf_DinosaurLODLevel::HighDetail:
                SkeletalMesh->SetForcedLOD(0);
                SkeletalMesh->SetVisibility(true);
                SkeletalMesh->bPauseAnims = false;
                break;
                
            case EPerf_DinosaurLODLevel::MediumDetail:
                SkeletalMesh->SetForcedLOD(1);
                SkeletalMesh->SetVisibility(true);
                SkeletalMesh->bPauseAnims = LODSettings.bEnableAnimationCulling;
                break;
                
            case EPerf_DinosaurLODLevel::LowDetail:
                SkeletalMesh->SetForcedLOD(2);
                SkeletalMesh->SetVisibility(true);
                SkeletalMesh->bPauseAnims = LODSettings.bEnableAnimationCulling;
                break;
                
            case EPerf_DinosaurLODLevel::Culled:
                SkeletalMesh->SetVisibility(false);
                SkeletalMesh->bPauseAnims = true;
                break;
        }
    }
    
    // Apply physics culling
    if (LODSettings.bEnablePhysicsCulling)
    {
        bool bEnableCollision = (LODLevel != EPerf_DinosaurLODLevel::Culled) && 
                               (LODLevel != EPerf_DinosaurLODLevel::LowDetail);
        DinosaurPawn->SetActorEnableCollision(bEnableCollision);
    }
}

float UPerf_DinosaurPerformanceManager::CalculateDistanceToPlayer(APawn* DinosaurPawn) const
{
    if (!DinosaurPawn)
    {
        return MAX_FLT;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return MAX_FLT;
    }
    
    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return MAX_FLT;
    }
    
    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    FVector DinosaurLocation = DinosaurPawn->GetActorLocation();
    
    return FVector::Dist(PlayerLocation, DinosaurLocation);
}

void UPerf_DinosaurPerformanceManager::LogPerformanceData()
{
    UE_LOG(LogTemp, Log, TEXT("Dinosaur Performance Stats:"));
    UE_LOG(LogTemp, Log, TEXT("  Total: %d, Visible: %d"), CurrentStats.TotalDinosaurs, CurrentStats.VisibleDinosaurs);
    UE_LOG(LogTemp, Log, TEXT("  High: %d, Medium: %d, Low: %d, Culled: %d"), 
           CurrentStats.HighDetailDinosaurs, CurrentStats.MediumDetailDinosaurs, 
           CurrentStats.LowDetailDinosaurs, CurrentStats.CulledDinosaurs);
    UE_LOG(LogTemp, Log, TEXT("  Avg Frame Time: %.2fms, Render Cost: %.1f"), 
           CurrentStats.AverageFrameTime * 1000.0f, CurrentStats.DinosaurRenderCost);
}

void UPerf_DinosaurPerformanceManager::CleanupInvalidReferences()
{
    RegisteredDinosaurs.RemoveAll([this](const TWeakObjectPtr<APawn>& DinosaurPtr)
    {
        if (!DinosaurPtr.IsValid())
        {
            DinosaurLODLevels.Remove(DinosaurPtr);
            return true;
        }
        return false;
    });
}

void UPerf_DinosaurPerformanceManager::SortDinosaursByDistance()
{
    RegisteredDinosaurs.Sort([this](const TWeakObjectPtr<APawn>& A, const TWeakObjectPtr<APawn>& B)
    {
        APawn* PawnA = A.Get();
        APawn* PawnB = B.Get();
        
        if (!PawnA || !PawnB)
        {
            return PawnA != nullptr;
        }
        
        float DistanceA = CalculateDistanceToPlayer(PawnA);
        float DistanceB = CalculateDistanceToPlayer(PawnB);
        
        return DistanceA < DistanceB;
    });
}

void UPerf_DinosaurPerformanceManager::ApplyVisibilityLimits()
{
    int32 VisibleCount = 0;
    
    for (const TWeakObjectPtr<APawn>& DinosaurPtr : RegisteredDinosaurs)
    {
        if (APawn* Dinosaur = DinosaurPtr.Get())
        {
            if (VisibleCount < LODSettings.MaxVisibleDinosaurs)
            {
                float Distance = CalculateDistanceToPlayer(Dinosaur);
                EPerf_DinosaurLODLevel LODLevel = DetermineLODLevel(Distance);
                
                if (LODLevel != EPerf_DinosaurLODLevel::Culled)
                {
                    VisibleCount++;
                }
            }
            else
            {
                // Force cull dinosaurs beyond the visibility limit
                SetDinosaurLODLevel(Dinosaur, EPerf_DinosaurLODLevel::Culled);
            }
        }
    }
}

EPerf_DinosaurLODLevel UPerf_DinosaurPerformanceManager::DetermineLODLevel(float Distance) const
{
    if (Distance > LODSettings.CullingDistance)
    {
        return EPerf_DinosaurLODLevel::Culled;
    }
    else if (Distance > LODSettings.LowDetailDistance)
    {
        return EPerf_DinosaurLODLevel::LowDetail;
    }
    else if (Distance > LODSettings.MediumDetailDistance)
    {
        return EPerf_DinosaurLODLevel::MediumDetail;
    }
    else
    {
        return EPerf_DinosaurLODLevel::HighDetail;
    }
}

bool UPerf_DinosaurPerformanceManager::IsDinosaurPawn(APawn* Pawn) const
{
    if (!Pawn)
    {
        return false;
    }
    
    FString PawnName = Pawn->GetName().ToLower();
    return PawnName.Contains(TEXT("rex")) || 
           PawnName.Contains(TEXT("raptor")) || 
           PawnName.Contains(TEXT("dino")) ||
           PawnName.Contains(TEXT("saur")) ||
           PawnName.Contains(TEXT("triceratops")) ||
           PawnName.Contains(TEXT("brachio")) ||
           PawnName.Contains(TEXT("ankylo"));
}