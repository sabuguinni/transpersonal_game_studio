#include "Quest_DinosaurHuntMission.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UQuest_DinosaurHuntMission::UQuest_DinosaurHuntMission()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize hunt parameters
    TrackingRange = 3000.0f;
    StealthDetectionRange = 1500.0f;
    AmbushRange = 500.0f;
    MissionTimeLimit = 1800.0f; // 30 minutes
    bAllowGroupHunt = false;
    bMissionActive = false;
    MissionStartTime = 0.0f;
}

void UQuest_DinosaurHuntMission::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize hunt progress
    HuntProgress.CurrentPhase = EQuest_HuntPhase::Tracking;
    HuntProgress.DinosaursSighted = 0;
    HuntProgress.DinosaursKilled = 0;
    HuntProgress.TrackingProgress = 0.0f;
    HuntProgress.StealthLevel = 100.0f;
    HuntProgress.bTargetDetected = false;
    HuntProgress.CurrentTarget = nullptr;
}

void UQuest_DinosaurHuntMission::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bMissionActive)
    {
        return;
    }

    // Update mission based on current phase
    switch (HuntProgress.CurrentPhase)
    {
        case EQuest_HuntPhase::Tracking:
            ProcessTrackingPhase(DeltaTime);
            break;
        case EQuest_HuntPhase::Stalking:
            ProcessStalkingPhase(DeltaTime);
            break;
        case EQuest_HuntPhase::Ambush:
            ProcessAmbushPhase(DeltaTime);
            break;
        case EQuest_HuntPhase::Combat:
            UpdateStealthDetection(DeltaTime);
            break;
        default:
            break;
    }

    // Check for nearby dinosaurs periodically
    CheckForNearbyDinosaurs();
}

void UQuest_DinosaurHuntMission::StartHuntMission(const FQuest_HuntTarget& Target)
{
    HuntTarget = Target;
    bMissionActive = true;
    MissionStartTime = GetWorld()->GetTimeSeconds();
    
    // Reset progress
    HuntProgress = FQuest_HuntProgress();
    HuntProgress.CurrentPhase = EQuest_HuntPhase::Tracking;
    
    // Set mission timer
    if (MissionTimeLimit > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(MissionTimerHandle, this, 
            &UQuest_DinosaurHuntMission::OnMissionTimeout, MissionTimeLimit, false);
    }

    // Start tracking timer
    GetWorld()->GetTimerManager().SetTimer(TrackingTimerHandle, this,
        &UQuest_DinosaurHuntMission::CheckForNearbyDinosaurs, 2.0f, true);

    UE_LOG(LogTemp, Warning, TEXT("Hunt Mission Started: Target %d dinosaurs of type %d"), 
        Target.RequiredCount, (int32)Target.DinosaurType);

    OnPhaseChanged.Broadcast(EQuest_HuntPhase::Tracking);
}

void UQuest_DinosaurHuntMission::UpdateTrackingProgress(float DeltaTime)
{
    if (HuntProgress.CurrentPhase != EQuest_HuntPhase::Tracking)
    {
        return;
    }

    // Increase tracking progress based on movement and observation
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        float MovementSpeed = PlayerPawn->GetVelocity().Size();
        float TrackingBonus = FMath::Clamp(MovementSpeed / 100.0f, 0.1f, 1.0f);
        
        HuntProgress.TrackingProgress += DeltaTime * TrackingBonus * 10.0f;
        HuntProgress.TrackingProgress = FMath::Clamp(HuntProgress.TrackingProgress, 0.0f, 100.0f);
    }
}

void UQuest_DinosaurHuntMission::CheckForNearbyDinosaurs()
{
    if (!bMissionActive)
    {
        return;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    NearbyDinosaurs.Empty();

    // Get all actors in the world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (!Actor || !IsValidHuntTarget(Actor))
        {
            continue;
        }

        float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
        if (Distance <= TrackingRange)
        {
            NearbyDinosaurs.Add(Actor);
            
            // Check if this is our first sighting of this type
            if (!HuntProgress.bTargetDetected)
            {
                HuntProgress.bTargetDetected = true;
                HuntProgress.DinosaursSighted++;
                OnTargetSighted.Broadcast(Actor);
                
                // Transition to stalking if close enough
                if (Distance <= StealthDetectionRange && HuntProgress.CurrentPhase == EQuest_HuntPhase::Tracking)
                {
                    EnterStalkingPhase(Actor);
                }
            }
        }
    }

    // Debug visualization
    if (GetWorld() && GetWorld()->IsGameWorld())
    {
        DrawDebugSphere(GetWorld(), PlayerLocation, TrackingRange, 32, FColor::Yellow, false, 2.0f);
        for (AActor* Dino : NearbyDinosaurs)
        {
            DrawDebugSphere(GetWorld(), Dino->GetActorLocation(), 100.0f, 16, FColor::Red, false, 2.0f);
        }
    }
}

void UQuest_DinosaurHuntMission::EnterStalkingPhase(AActor* Target)
{
    if (!Target)
    {
        return;
    }

    HuntProgress.CurrentTarget = Target;
    UpdateHuntPhase(EQuest_HuntPhase::Stalking);
    
    UE_LOG(LogTemp, Warning, TEXT("Entering Stalking Phase with target: %s"), *Target->GetName());
}

void UQuest_DinosaurHuntMission::AttemptAmbush()
{
    if (HuntProgress.CurrentPhase != EQuest_HuntPhase::Stalking || !HuntProgress.CurrentTarget)
    {
        return;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    float DistanceToTarget = FVector::Dist(PlayerPawn->GetActorLocation(), 
        HuntProgress.CurrentTarget->GetActorLocation());

    if (DistanceToTarget <= AmbushRange && HuntProgress.StealthLevel > 50.0f)
    {
        UpdateHuntPhase(EQuest_HuntPhase::Ambush);
        UE_LOG(LogTemp, Warning, TEXT("Ambush initiated! Distance: %.2f, Stealth: %.2f"), 
            DistanceToTarget, HuntProgress.StealthLevel);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Ambush failed - Distance: %.2f, Stealth: %.2f"), 
            DistanceToTarget, HuntProgress.StealthLevel);
        FailMission(TEXT("Ambush attempt failed - too far or detected"));
    }
}

void UQuest_DinosaurHuntMission::OnDinosaurKilled(AActor* KilledDinosaur)
{
    if (!KilledDinosaur || !IsValidHuntTarget(KilledDinosaur))
    {
        return;
    }

    HuntProgress.DinosaursKilled++;
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur killed! Progress: %d/%d"), 
        HuntProgress.DinosaursKilled, HuntTarget.RequiredCount);

    if (HuntProgress.DinosaursKilled >= HuntTarget.RequiredCount)
    {
        UpdateHuntPhase(EQuest_HuntPhase::Harvest);
        
        // Auto-complete after harvest phase
        GetWorld()->GetTimerManager().SetTimer(MissionTimerHandle, this,
            &UQuest_DinosaurHuntMission::CompleteMission, 5.0f, false);
    }
}

void UQuest_DinosaurHuntMission::CompleteMission()
{
    bMissionActive = false;
    UpdateHuntPhase(EQuest_HuntPhase::Completed);
    
    // Clear timers
    GetWorld()->GetTimerManager().ClearTimer(MissionTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(TrackingTimerHandle);
    
    UE_LOG(LogTemp, Warning, TEXT("Hunt Mission Completed Successfully!"));
    OnMissionCompleted.Broadcast();
}

void UQuest_DinosaurHuntMission::FailMission(const FString& Reason)
{
    bMissionActive = false;
    UpdateHuntPhase(EQuest_HuntPhase::Failed);
    
    // Clear timers
    GetWorld()->GetTimerManager().ClearTimer(MissionTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(TrackingTimerHandle);
    
    UE_LOG(LogTemp, Error, TEXT("Hunt Mission Failed: %s"), *Reason);
    OnMissionFailed.Broadcast();
}

float UQuest_DinosaurHuntMission::CalculateStealthLevel() const
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return 0.0f;
    }

    float BaseStealthLevel = 100.0f;
    float MovementPenalty = FMath::Clamp(PlayerPawn->GetVelocity().Size() / 10.0f, 0.0f, 50.0f);
    
    // Reduce stealth based on proximity to dinosaurs
    for (AActor* Dino : NearbyDinosaurs)
    {
        if (Dino)
        {
            float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Dino->GetActorLocation());
            float ProximityPenalty = FMath::Clamp((1500.0f - Distance) / 30.0f, 0.0f, 30.0f);
            BaseStealthLevel -= ProximityPenalty;
        }
    }

    return FMath::Clamp(BaseStealthLevel - MovementPenalty, 0.0f, 100.0f);
}

bool UQuest_DinosaurHuntMission::IsValidHuntTarget(AActor* Dinosaur) const
{
    if (!Dinosaur)
    {
        return false;
    }

    FString ActorName = Dinosaur->GetActorLabel().ToLower();
    
    // Check if actor name contains dinosaur keywords
    switch (HuntTarget.DinosaurType)
    {
        case EQuest_DinosaurType::TRex:
            return ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("t-rex"));
        case EQuest_DinosaurType::Velociraptor:
            return ActorName.Contains(TEXT("veloci")) || ActorName.Contains(TEXT("raptor"));
        case EQuest_DinosaurType::Triceratops:
            return ActorName.Contains(TEXT("tricera"));
        case EQuest_DinosaurType::Brachiosaurus:
            return ActorName.Contains(TEXT("brachi"));
        case EQuest_DinosaurType::Ankylosaurus:
            return ActorName.Contains(TEXT("ankylo"));
        case EQuest_DinosaurType::Parasaurolophus:
            return ActorName.Contains(TEXT("parasauro"));
        default:
            return false;
    }
}

FVector UQuest_DinosaurHuntMission::GetOptimalAmbushPosition(AActor* Target) const
{
    if (!Target)
    {
        return FVector::ZeroVector;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return FVector::ZeroVector;
    }

    FVector TargetLocation = Target->GetActorLocation();
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - PlayerLocation).GetSafeNormal();
    
    // Position behind and to the side of the target
    FVector AmbushOffset = FVector::CrossProduct(DirectionToTarget, FVector::UpVector) * 300.0f;
    AmbushOffset += DirectionToTarget * -200.0f; // Behind the target
    
    return TargetLocation + AmbushOffset;
}

void UQuest_DinosaurHuntMission::UpdateHuntPhase(EQuest_HuntPhase NewPhase)
{
    if (HuntProgress.CurrentPhase != NewPhase)
    {
        HuntProgress.CurrentPhase = NewPhase;
        OnPhaseChanged.Broadcast(NewPhase);
        
        UE_LOG(LogTemp, Warning, TEXT("Hunt Phase Changed to: %d"), (int32)NewPhase);
    }
}

float UQuest_DinosaurHuntMission::GetMissionProgress() const
{
    if (!bMissionActive)
    {
        return HuntProgress.CurrentPhase == EQuest_HuntPhase::Completed ? 100.0f : 0.0f;
    }

    float PhaseProgress = 0.0f;
    switch (HuntProgress.CurrentPhase)
    {
        case EQuest_HuntPhase::Tracking:
            PhaseProgress = HuntProgress.TrackingProgress * 0.2f; // 20% for tracking
            break;
        case EQuest_HuntPhase::Stalking:
            PhaseProgress = 20.0f + (HuntProgress.StealthLevel * 0.3f); // 20-50%
            break;
        case EQuest_HuntPhase::Ambush:
            PhaseProgress = 50.0f + 20.0f; // 70%
            break;
        case EQuest_HuntPhase::Combat:
            PhaseProgress = 70.0f + 15.0f; // 85%
            break;
        case EQuest_HuntPhase::Harvest:
            PhaseProgress = 85.0f + 10.0f; // 95%
            break;
        case EQuest_HuntPhase::Completed:
            PhaseProgress = 100.0f;
            break;
        default:
            PhaseProgress = 0.0f;
            break;
    }

    return FMath::Clamp(PhaseProgress, 0.0f, 100.0f);
}

bool UQuest_DinosaurHuntMission::IsMissionActive() const
{
    return bMissionActive && HuntProgress.CurrentPhase != EQuest_HuntPhase::Completed 
        && HuntProgress.CurrentPhase != EQuest_HuntPhase::Failed;
}

void UQuest_DinosaurHuntMission::OnMissionTimeout()
{
    FailMission(TEXT("Mission time limit exceeded"));
}

void UQuest_DinosaurHuntMission::UpdateStealthDetection(float DeltaTime)
{
    HuntProgress.StealthLevel = CalculateStealthLevel();
    
    // If stealth drops too low, mission may fail
    if (HuntProgress.StealthLevel < 10.0f && HuntProgress.CurrentPhase == EQuest_HuntPhase::Stalking)
    {
        FailMission(TEXT("Detected by target - stealth compromised"));
    }
}

void UQuest_DinosaurHuntMission::ProcessTrackingPhase(float DeltaTime)
{
    UpdateTrackingProgress(DeltaTime);
    
    // Check if we can advance to stalking
    if (HuntProgress.bTargetDetected && HuntProgress.TrackingProgress > 50.0f)
    {
        for (AActor* Dino : NearbyDinosaurs)
        {
            if (Dino && IsValidHuntTarget(Dino))
            {
                APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
                if (PlayerPawn)
                {
                    float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Dino->GetActorLocation());
                    if (Distance <= StealthDetectionRange)
                    {
                        EnterStalkingPhase(Dino);
                        break;
                    }
                }
            }
        }
    }
}

void UQuest_DinosaurHuntMission::ProcessStalkingPhase(float DeltaTime)
{
    UpdateStealthDetection(DeltaTime);
    
    if (!HuntProgress.CurrentTarget)
    {
        // Lost target, return to tracking
        UpdateHuntPhase(EQuest_HuntPhase::Tracking);
        return;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        float DistanceToTarget = FVector::Dist(PlayerPawn->GetActorLocation(), 
            HuntProgress.CurrentTarget->GetActorLocation());
            
        if (DistanceToTarget <= AmbushRange && HuntProgress.StealthLevel > 70.0f)
        {
            // Auto-attempt ambush when in optimal position
            AttemptAmbush();
        }
        else if (DistanceToTarget > StealthDetectionRange * 1.5f)
        {
            // Target too far, return to tracking
            UpdateHuntPhase(EQuest_HuntPhase::Tracking);
        }
    }
}

void UQuest_DinosaurHuntMission::ProcessAmbushPhase(float DeltaTime)
{
    // Ambush phase automatically transitions to combat
    // This could trigger combat mechanics or damage to the target
    UpdateHuntPhase(EQuest_HuntPhase::Combat);
    
    // For now, simulate successful hunt after short delay
    GetWorld()->GetTimerManager().SetTimer(MissionTimerHandle, [this]()
    {
        if (HuntProgress.CurrentTarget)
        {
            OnDinosaurKilled(HuntProgress.CurrentTarget);
        }
    }, 3.0f, false);
}