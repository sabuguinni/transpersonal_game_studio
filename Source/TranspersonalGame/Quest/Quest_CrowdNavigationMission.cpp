#include "Quest_CrowdNavigationMission.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "DrawDebugHelpers.h"

AQuest_CrowdNavigationMission::AQuest_CrowdNavigationMission()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create mission marker
    MissionMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MissionMarker"));
    MissionMarker->SetupAttachment(RootComponent);

    // Create detection sphere
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
    DetectionSphere->SetSphereRadius(500.0f);
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create completion sphere
    CompletionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CompletionSphere"));
    CompletionSphere->SetupAttachment(RootComponent);
    CompletionSphere->SetSphereRadius(100.0f);
    CompletionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CompletionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    CompletionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Initialize default values
    bMissionActive = false;
    bPlayerDetected = false;
    DetectionCooldown = 3.0f;
    DetectionTimer = 0.0f;
    CurrentObjectiveIndex = 0;
    bObjectiveCompleted = false;
    CurrentCrowdDensity = 0;
    CrowdDensityCheckRadius = 1000.0f;
    CrowdDensityUpdateTimer = 0.0f;
    LastPlayerPosition = FVector::ZeroVector;

    // Set default objective
    CurrentObjective.TargetLocation = GetActorLocation();
    CurrentObjective.NavigationType = EQuest_CrowdNavigationType::Stealth;
    CurrentObjective.DetectionRadius = 500.0f;
    CurrentObjective.CompletionRadius = 100.0f;
    CurrentObjective.bRequiresCrowdDensity = true;
    CurrentObjective.MinimumCrowdSize = 10;
}

void AQuest_CrowdNavigationMission::BeginPlay()
{
    Super::BeginPlay();

    // Initialize mission marker visual
    if (MissionMarker)
    {
        // Load default cube mesh if available
        UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
        if (CubeMesh)
        {
            MissionMarker->SetStaticMesh(CubeMesh);
            MissionMarker->SetWorldScale3D(FVector(0.5f, 0.5f, 2.0f));
        }
    }

    // Set up sphere radii based on current objective
    if (DetectionSphere)
    {
        DetectionSphere->SetSphereRadius(CurrentObjective.DetectionRadius);
    }

    if (CompletionSphere)
    {
        CompletionSphere->SetSphereRadius(CurrentObjective.CompletionRadius);
    }

    // Initialize crowd density check
    UpdateCrowdDensity();
}

void AQuest_CrowdNavigationMission::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bMissionActive)
    {
        return;
    }

    // Update detection timer
    if (DetectionTimer > 0.0f)
    {
        DetectionTimer -= DeltaTime;
        if (DetectionTimer <= 0.0f)
        {
            bPlayerDetected = false;
        }
    }

    // Update crowd density periodically
    CrowdDensityUpdateTimer += DeltaTime;
    if (CrowdDensityUpdateTimer >= 2.0f)
    {
        UpdateCrowdDensity();
        CrowdDensityUpdateTimer = 0.0f;
    }

    // Check for player detection
    CheckPlayerDetection();

    // Check objective completion
    if (CheckObjectiveCompletion())
    {
        SetNextObjective();
    }

    // Update navigation hints
    UpdateNavigationHints();

    // Debug visualization
    if (GEngine && GEngine->bEnableOnScreenDebugMessages)
    {
        DrawDebugSphere(GetWorld(), GetActorLocation(), CurrentObjective.DetectionRadius, 12, FColor::Yellow, false, 0.1f);
        DrawDebugSphere(GetWorld(), GetActorLocation(), CurrentObjective.CompletionRadius, 12, FColor::Green, false, 0.1f);
        
        FString DebugText = FString::Printf(TEXT("Crowd Mission - Density: %d, Detected: %s"), 
            CurrentCrowdDensity, bPlayerDetected ? TEXT("YES") : TEXT("NO"));
        DrawDebugString(GetWorld(), GetActorLocation() + FVector(0, 0, 150), DebugText, nullptr, FColor::White, 0.1f);
    }
}

void AQuest_CrowdNavigationMission::StartCrowdNavigationMission()
{
    bMissionActive = true;
    CurrentObjectiveIndex = 0;
    bObjectiveCompleted = false;
    bPlayerDetected = false;
    DetectionTimer = 0.0f;

    if (NavigationObjectives.Num() > 0)
    {
        CurrentObjective = NavigationObjectives[0];
    }

    // Update sphere components
    if (DetectionSphere)
    {
        DetectionSphere->SetSphereRadius(CurrentObjective.DetectionRadius);
    }

    if (CompletionSphere)
    {
        CompletionSphere->SetSphereRadius(CurrentObjective.CompletionRadius);
    }

    OnMissionStarted();

    UE_LOG(LogTemp, Warning, TEXT("Crowd Navigation Mission Started - Type: %d"), 
        (int32)CurrentObjective.NavigationType);
}

void AQuest_CrowdNavigationMission::CompleteMission()
{
    bMissionActive = false;
    bObjectiveCompleted = true;
    
    OnMissionCompleted();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Navigation Mission Completed Successfully"));
}

void AQuest_CrowdNavigationMission::FailMission()
{
    bMissionActive = false;
    bObjectiveCompleted = false;
    
    OnMissionFailed();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Navigation Mission Failed"));
}

void AQuest_CrowdNavigationMission::SetNextObjective()
{
    CurrentObjectiveIndex++;
    
    if (CurrentObjectiveIndex >= NavigationObjectives.Num())
    {
        CompleteMission();
        return;
    }

    CurrentObjective = NavigationObjectives[CurrentObjectiveIndex];
    bObjectiveCompleted = false;

    // Update sphere components for new objective
    if (DetectionSphere)
    {
        DetectionSphere->SetSphereRadius(CurrentObjective.DetectionRadius);
    }

    if (CompletionSphere)
    {
        CompletionSphere->SetSphereRadius(CurrentObjective.CompletionRadius);
    }

    OnObjectiveCompleted();

    UE_LOG(LogTemp, Warning, TEXT("Moving to next objective: %d"), CurrentObjectiveIndex);
}

bool AQuest_CrowdNavigationMission::CheckObjectiveCompletion()
{
    if (bObjectiveCompleted || !bMissionActive)
    {
        return false;
    }

    // Get player pawn
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return false;
    }

    float DistanceToTarget = FVector::Dist(PlayerPawn->GetActorLocation(), CurrentObjective.TargetLocation);
    
    // Check if player is within completion radius
    if (DistanceToTarget <= CurrentObjective.CompletionRadius)
    {
        // Check crowd density requirement
        if (CurrentObjective.bRequiresCrowdDensity)
        {
            if (CurrentCrowdDensity < CurrentObjective.MinimumCrowdSize)
            {
                return false;
            }
        }

        // Check navigation type specific requirements
        switch (CurrentObjective.NavigationType)
        {
            case EQuest_CrowdNavigationType::Stealth:
                // Must not be detected
                if (bPlayerDetected)
                {
                    return false;
                }
                break;

            case EQuest_CrowdNavigationType::Following:
                // Must be near crowd actors
                if (!IsPlayerInCrowd())
                {
                    return false;
                }
                break;

            case EQuest_CrowdNavigationType::Infiltration:
                // Must be within crowd and undetected
                if (!IsPlayerInCrowd() || bPlayerDetected)
                {
                    return false;
                }
                break;

            case EQuest_CrowdNavigationType::Escape:
                // Must have used crowd for cover
                if (CurrentCrowdDensity < 5)
                {
                    return false;
                }
                break;

            case EQuest_CrowdNavigationType::Observation:
                // Must maintain position for observation
                if (FVector::Dist(PlayerPawn->GetActorLocation(), LastPlayerPosition) > 50.0f)
                {
                    LastPlayerPosition = PlayerPawn->GetActorLocation();
                    return false;
                }
                break;
        }

        bObjectiveCompleted = true;
        return true;
    }

    return false;
}

void AQuest_CrowdNavigationMission::UpdateCrowdDensity()
{
    NearbyCrowdActors = GetNearbyCrowdActors(CrowdDensityCheckRadius);
    CurrentCrowdDensity = NearbyCrowdActors.Num();

    UE_LOG(LogTemp, Log, TEXT("Updated crowd density: %d actors within %f units"), 
        CurrentCrowdDensity, CrowdDensityCheckRadius);
}

bool AQuest_CrowdNavigationMission::IsPlayerInCrowd()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return false;
    }

    TArray<AActor*> NearbyActors = GetNearbyCrowdActors(200.0f);
    return NearbyActors.Num() >= 3; // Player is "in crowd" if 3+ crowd actors nearby
}

void AQuest_CrowdNavigationMission::CheckPlayerDetection()
{
    if (bPlayerDetected && DetectionTimer > 0.0f)
    {
        return; // Still in detection cooldown
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    float DistanceToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
    
    // Check if player is within detection range
    if (DistanceToPlayer <= CurrentObjective.DetectionRadius)
    {
        // Detection logic based on navigation type
        bool bShouldDetect = false;

        switch (CurrentObjective.NavigationType)
        {
            case EQuest_CrowdNavigationType::Stealth:
                // Detect if not in crowd
                bShouldDetect = !IsPlayerInCrowd();
                break;

            case EQuest_CrowdNavigationType::Infiltration:
                // Detect if moving too fast or not blending
                bShouldDetect = (PlayerPawn->GetVelocity().Size() > 300.0f) && !IsPlayerInCrowd();
                break;

            default:
                bShouldDetect = false;
                break;
        }

        if (bShouldDetect && !bPlayerDetected)
        {
            bPlayerDetected = true;
            DetectionTimer = DetectionCooldown;
            OnPlayerDetected();

            UE_LOG(LogTemp, Warning, TEXT("Player detected! Detection timer started."));
        }
    }
}

TArray<AActor*> AQuest_CrowdNavigationMission::GetNearbyCrowdActors(float SearchRadius)
{
    TArray<AActor*> FoundActors;
    TArray<AActor*> AllActors;

    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (!Actor || Actor == this)
        {
            continue;
        }

        // Check if actor has "Crowd" in its name or class
        FString ActorName = Actor->GetName();
        FString ClassName = Actor->GetClass()->GetName();
        
        if (ActorName.Contains(TEXT("Crowd")) || ClassName.Contains(TEXT("Crowd")) || 
            ActorName.Contains(TEXT("NPC")) || ClassName.Contains(TEXT("Character")))
        {
            float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
            if (Distance <= SearchRadius)
            {
                FoundActors.Add(Actor);
            }
        }
    }

    return FoundActors;
}

void AQuest_CrowdNavigationMission::UpdateNavigationHints()
{
    // Provide navigation hints to player based on crowd positions
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    FVector OptimalPath = GetOptimalCrowdPath();
    
    // Visual hint: draw line to optimal path
    if (GEngine && GEngine->bEnableOnScreenDebugMessages)
    {
        DrawDebugLine(GetWorld(), PlayerPawn->GetActorLocation(), OptimalPath, 
            FColor::Blue, false, 0.1f, 0, 2.0f);
    }
}

FVector AQuest_CrowdNavigationMission::GetOptimalCrowdPath()
{
    // Calculate optimal path through crowd areas
    FVector PlayerLocation = UGameplayStatics::GetPlayerPawn(GetWorld(), 0)->GetActorLocation();
    FVector TargetLocation = CurrentObjective.TargetLocation;
    
    // Find densest crowd area between player and target
    FVector BestPath = TargetLocation;
    float BestCrowdDensity = 0.0f;

    // Sample points along the path
    for (int32 i = 1; i < 10; i++)
    {
        float Alpha = i / 10.0f;
        FVector SamplePoint = FMath::Lerp(PlayerLocation, TargetLocation, Alpha);
        
        TArray<AActor*> LocalCrowd = GetNearbyCrowdActors(300.0f);
        float LocalDensity = 0.0f;
        
        for (AActor* CrowdActor : LocalCrowd)
        {
            float Distance = FVector::Dist(SamplePoint, CrowdActor->GetActorLocation());
            if (Distance < 200.0f)
            {
                LocalDensity += (200.0f - Distance) / 200.0f; // Weighted by proximity
            }
        }

        if (LocalDensity > BestCrowdDensity)
        {
            BestCrowdDensity = LocalDensity;
            BestPath = SamplePoint;
        }
    }

    return BestPath;
}

bool AQuest_CrowdNavigationMission::ValidateNavigationRoute()
{
    // Validate that the current route has sufficient crowd cover
    FVector OptimalPath = GetOptimalCrowdPath();
    TArray<AActor*> PathCrowd = GetNearbyCrowdActors(500.0f);
    
    return PathCrowd.Num() >= CurrentObjective.MinimumCrowdSize;
}