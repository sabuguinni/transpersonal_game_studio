#include "Combat_ThreatDetectionSystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"

UCombat_ThreatDetectionSystem::UCombat_ThreatDetectionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Check every 0.5 seconds
    
    ThreatDetectionRange = 5000.0f; // 50 meters
    CombatRange = 2000.0f; // 20 meters
    AlertRange = 3500.0f; // 35 meters
    
    CurrentThreatLevel = ECombat_ThreatLevel::None;
    bIsInCombat = false;
    bDebugMode = false;
}

void UCombat_ThreatDetectionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn)
    {
        UE_LOG(LogTemp, Error, TEXT("ThreatDetectionSystem: Owner is not a Pawn!"));
        return;
    }
    
    // Initialize threat detection
    DetectedThreats.Empty();
    LastThreatScanTime = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("ThreatDetectionSystem initialized for %s"), *OwnerPawn->GetName());
}

void UCombat_ThreatDetectionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerPawn)
        return;
    
    // Perform threat detection scan
    ScanForThreats();
    
    // Update threat level based on detected threats
    UpdateThreatLevel();
    
    // Handle combat state changes
    HandleCombatStateChanges();
    
    // Debug visualization
    if (bDebugMode)
    {
        DrawDebugInfo();
    }
}

void UCombat_ThreatDetectionSystem::ScanForThreats()
{
    if (!OwnerPawn || !GetWorld())
        return;
    
    FVector OwnerLocation = OwnerPawn->GetActorLocation();
    DetectedThreats.Empty();
    
    // Get all pawns in the world
    TArray<AActor*> AllPawns;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllPawns);
    
    for (AActor* Actor : AllPawns)
    {
        APawn* OtherPawn = Cast<APawn>(Actor);
        if (!OtherPawn || OtherPawn == OwnerPawn)
            continue;
        
        // Check if this pawn is a potential threat
        if (IsActorAThreat(OtherPawn))
        {
            float Distance = FVector::Dist(OwnerLocation, OtherPawn->GetActorLocation());
            
            if (Distance <= ThreatDetectionRange)
            {
                FCombat_ThreatInfo ThreatInfo;
                ThreatInfo.ThreatActor = OtherPawn;
                ThreatInfo.Distance = Distance;
                ThreatInfo.LastSeenLocation = OtherPawn->GetActorLocation();
                ThreatInfo.ThreatType = DetermineThreatType(OtherPawn);
                ThreatInfo.LastDetectionTime = GetWorld()->GetTimeSeconds();
                
                DetectedThreats.Add(ThreatInfo);
                
                if (bDebugMode)
                {
                    UE_LOG(LogTemp, Log, TEXT("Threat detected: %s at distance %.1f"), 
                           *OtherPawn->GetName(), Distance);
                }
            }
        }
    }
    
    // Sort threats by distance (closest first)
    DetectedThreats.Sort([](const FCombat_ThreatInfo& A, const FCombat_ThreatInfo& B)
    {
        return A.Distance < B.Distance;
    });
    
    LastThreatScanTime = GetWorld()->GetTimeSeconds();
}

bool UCombat_ThreatDetectionSystem::IsActorAThreat(AActor* Actor) const
{
    if (!Actor)
        return false;
    
    // Check if actor has "dinosaur", "enemy", or other threat keywords in name
    FString ActorName = Actor->GetName().ToLower();
    
    TArray<FString> ThreatKeywords = {
        TEXT("dinosaur"), TEXT("trex"), TEXT("raptor"), TEXT("brachio"),
        TEXT("enemy"), TEXT("hostile"), TEXT("predator"), TEXT("carnivore")
    };
    
    for (const FString& Keyword : ThreatKeywords)
    {
        if (ActorName.Contains(Keyword))
        {
            return true;
        }
    }
    
    // Check if actor has specific threat components or tags
    if (Actor->Tags.Contains(TEXT("Threat")) || Actor->Tags.Contains(TEXT("Enemy")))
    {
        return true;
    }
    
    return false;
}

ECombat_ThreatType UCombat_ThreatDetectionSystem::DetermineThreatType(AActor* ThreatActor) const
{
    if (!ThreatActor)
        return ECombat_ThreatType::Unknown;
    
    FString ActorName = ThreatActor->GetName().ToLower();
    
    if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("tyrannosaurus")))
    {
        return ECombat_ThreatType::LargePredator;
    }
    else if (ActorName.Contains(TEXT("raptor")) || ActorName.Contains(TEXT("velociraptor")))
    {
        return ECombat_ThreatType::PackHunter;
    }
    else if (ActorName.Contains(TEXT("brachio")) || ActorName.Contains(TEXT("herbivore")))
    {
        return ECombat_ThreatType::LargeHerbivore;
    }
    else if (ActorName.Contains(TEXT("small")) || ActorName.Contains(TEXT("compy")))
    {
        return ECombat_ThreatType::SmallThreat;
    }
    
    return ECombat_ThreatType::Unknown;
}

void UCombat_ThreatDetectionSystem::UpdateThreatLevel()
{
    ECombat_ThreatLevel NewThreatLevel = ECombat_ThreatLevel::None;
    
    if (DetectedThreats.Num() == 0)
    {
        NewThreatLevel = ECombat_ThreatLevel::None;
    }
    else
    {
        // Get closest threat
        const FCombat_ThreatInfo& ClosestThreat = DetectedThreats[0];
        
        if (ClosestThreat.Distance <= CombatRange)
        {
            NewThreatLevel = ECombat_ThreatLevel::Critical;
        }
        else if (ClosestThreat.Distance <= AlertRange)
        {
            NewThreatLevel = ECombat_ThreatLevel::High;
        }
        else
        {
            NewThreatLevel = ECombat_ThreatLevel::Medium;
        }
        
        // Escalate threat level based on number and type of threats
        if (DetectedThreats.Num() >= 3)
        {
            NewThreatLevel = ECombat_ThreatLevel::Critical;
        }
        else if (DetectedThreats.Num() >= 2 && NewThreatLevel == ECombat_ThreatLevel::High)
        {
            NewThreatLevel = ECombat_ThreatLevel::Critical;
        }
    }
    
    // Update threat level if changed
    if (NewThreatLevel != CurrentThreatLevel)
    {
        ECombat_ThreatLevel OldThreatLevel = CurrentThreatLevel;
        CurrentThreatLevel = NewThreatLevel;
        
        OnThreatLevelChanged.Broadcast(OldThreatLevel, CurrentThreatLevel);
        
        if (bDebugMode)
        {
            UE_LOG(LogTemp, Log, TEXT("Threat level changed from %d to %d"), 
                   (int32)OldThreatLevel, (int32)CurrentThreatLevel);
        }
    }
}

void UCombat_ThreatDetectionSystem::HandleCombatStateChanges()
{
    bool bShouldBeInCombat = (CurrentThreatLevel == ECombat_ThreatLevel::Critical);
    
    if (bShouldBeInCombat != bIsInCombat)
    {
        bIsInCombat = bShouldBeInCombat;
        
        if (bIsInCombat)
        {
            OnCombatStarted.Broadcast();
            UE_LOG(LogTemp, Warning, TEXT("COMBAT STARTED for %s"), *OwnerPawn->GetName());
        }
        else
        {
            OnCombatEnded.Broadcast();
            UE_LOG(LogTemp, Log, TEXT("Combat ended for %s"), *OwnerPawn->GetName());
        }
    }
}

FCombat_ThreatInfo UCombat_ThreatDetectionSystem::GetClosestThreat() const
{
    if (DetectedThreats.Num() > 0)
    {
        return DetectedThreats[0];
    }
    
    return FCombat_ThreatInfo();
}

TArray<FCombat_ThreatInfo> UCombat_ThreatDetectionSystem::GetThreatsInRange(float Range) const
{
    TArray<FCombat_ThreatInfo> ThreatsInRange;
    
    for (const FCombat_ThreatInfo& Threat : DetectedThreats)
    {
        if (Threat.Distance <= Range)
        {
            ThreatsInRange.Add(Threat);
        }
    }
    
    return ThreatsInRange;
}

void UCombat_ThreatDetectionSystem::SetThreatDetectionRange(float NewRange)
{
    ThreatDetectionRange = FMath::Clamp(NewRange, 100.0f, 10000.0f);
}

void UCombat_ThreatDetectionSystem::DrawDebugInfo()
{
    if (!OwnerPawn || !GetWorld())
        return;
    
    FVector OwnerLocation = OwnerPawn->GetActorLocation();
    
    // Draw detection range
    DrawDebugSphere(GetWorld(), OwnerLocation, ThreatDetectionRange, 32, FColor::Yellow, false, 0.6f, 0, 2.0f);
    
    // Draw alert range
    DrawDebugSphere(GetWorld(), OwnerLocation, AlertRange, 32, FColor::Orange, false, 0.6f, 0, 2.0f);
    
    // Draw combat range
    DrawDebugSphere(GetWorld(), OwnerLocation, CombatRange, 32, FColor::Red, false, 0.6f, 0, 3.0f);
    
    // Draw lines to detected threats
    for (const FCombat_ThreatInfo& Threat : DetectedThreats)
    {
        if (IsValid(Threat.ThreatActor))
        {
            FColor LineColor = FColor::Green;
            
            if (Threat.Distance <= CombatRange)
                LineColor = FColor::Red;
            else if (Threat.Distance <= AlertRange)
                LineColor = FColor::Orange;
            
            DrawDebugLine(GetWorld(), OwnerLocation, Threat.ThreatActor->GetActorLocation(), 
                         LineColor, false, 0.6f, 0, 2.0f);
            
            // Draw threat info
            FString ThreatText = FString::Printf(TEXT("%s\nDist: %.1f\nType: %d"), 
                                               *Threat.ThreatActor->GetName(), 
                                               Threat.Distance,
                                               (int32)Threat.ThreatType);
            
            DrawDebugString(GetWorld(), Threat.ThreatActor->GetActorLocation() + FVector(0, 0, 100), 
                           ThreatText, nullptr, LineColor, 0.6f);
        }
    }
}