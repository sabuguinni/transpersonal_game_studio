#include "Combat_ThreatDetectionSystem.h"
#include "TranspersonalCharacter.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"

UCombat_ThreatDetectionSystem::UCombat_ThreatDetectionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    ScanRadius = 5000.0f;
    ScanInterval = 0.5f;
    CurrentThreatLevel = EThreatLevel::None;
    LastScanTime = 0.0f;
    PlayerCharacter = nullptr;

    // Initialize threat database with default dinosaur threats
    FCombat_ThreatLevel TRexThreat;
    TRexThreat.ThreatName = TEXT("T-Rex");
    TRexThreat.Level = EThreatLevel::Extreme;
    TRexThreat.ThreatRadius = 2000.0f;
    TRexThreat.DamagePerSecond = 100.0f;
    ThreatDatabase.Add(TRexThreat);

    FCombat_ThreatLevel RaptorThreat;
    RaptorThreat.ThreatName = TEXT("Velociraptor");
    RaptorThreat.Level = EThreatLevel::High;
    RaptorThreat.ThreatRadius = 800.0f;
    RaptorThreat.DamagePerSecond = 30.0f;
    ThreatDatabase.Add(RaptorThreat);

    FCombat_ThreatLevel TriceratopsThreat;
    TriceratopsThreat.ThreatName = TEXT("Triceratops");
    TriceratopsThreat.Level = EThreatLevel::Medium;
    TriceratopsThreat.ThreatRadius = 1200.0f;
    TriceratopsThreat.DamagePerSecond = 50.0f;
    ThreatDatabase.Add(TriceratopsThreat);
}

void UCombat_ThreatDetectionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Find player character
    PlayerCharacter = Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!PlayerCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("Combat_ThreatDetectionSystem: Player character not found"));
    }
    
    // Start initial scan
    ScanForThreats();
}

void UCombat_ThreatDetectionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!PlayerCharacter)
    {
        return;
    }
    
    // Periodic threat scanning
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastScanTime >= ScanInterval)
    {
        ScanForThreats();
        LastScanTime = CurrentTime;
    }
    
    // Update threat distances and remove lost threats
    for (int32 i = ActiveThreats.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(ActiveThreats[i].ThreatActor))
        {
            OnThreatLost(ActiveThreats[i].ThreatActor);
            ActiveThreats.RemoveAt(i);
            continue;
        }
        
        FVector PlayerLocation = PlayerCharacter->GetActorLocation();
        FVector ThreatLocation = ActiveThreats[i].ThreatActor->GetActorLocation();
        float Distance = FVector::Dist(PlayerLocation, ThreatLocation);
        
        ActiveThreats[i].Distance = Distance;
        ActiveThreats[i].Direction = (ThreatLocation - PlayerLocation).GetSafeNormal();
        
        // Remove threats that moved too far away
        FCombat_ThreatLevel ThreatLevel = GetThreatLevel(ActiveThreats[i].ThreatActor);
        if (Distance > ThreatLevel.ThreatRadius * 1.5f)
        {
            OnThreatLost(ActiveThreats[i].ThreatActor);
            ActiveThreats.RemoveAt(i);
        }
    }
    
    // Update current threat level
    EThreatLevel NewThreatLevel = EThreatLevel::None;
    for (const FCombat_DetectionData& Threat : ActiveThreats)
    {
        FCombat_ThreatLevel ThreatLevel = GetThreatLevel(Threat.ThreatActor);
        if (ThreatLevel.Level > NewThreatLevel)
        {
            NewThreatLevel = ThreatLevel.Level;
        }
    }
    
    if (NewThreatLevel != CurrentThreatLevel)
    {
        CurrentThreatLevel = NewThreatLevel;
        OnThreatLevelChanged(CurrentThreatLevel);
    }
}

void UCombat_ThreatDetectionSystem::ScanForThreats()
{
    if (!PlayerCharacter)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    TArray<AActor*> FoundActors;
    
    // Get all actors within scan radius
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        PlayerLocation,
        ScanRadius,
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        nullptr,
        TArray<AActor*>{PlayerCharacter},
        FoundActors
    );
    
    for (AActor* Actor : FoundActors)
    {
        if (!IsValid(Actor) || !IsActorThreat(Actor))
        {
            continue;
        }
        
        // Check if already detected
        bool bAlreadyDetected = false;
        for (const FCombat_DetectionData& ExistingThreat : ActiveThreats)
        {
            if (ExistingThreat.ThreatActor == Actor)
            {
                bAlreadyDetected = true;
                break;
            }
        }
        
        if (!bAlreadyDetected)
        {
            // New threat detected
            FCombat_DetectionData NewThreat;
            NewThreat.ThreatActor = Actor;
            NewThreat.Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            NewThreat.Direction = (Actor->GetActorLocation() - PlayerLocation).GetSafeNormal();
            NewThreat.DetectionTime = GetWorld()->GetTimeSeconds();
            
            ActiveThreats.Add(NewThreat);
            OnThreatDetected(NewThreat);
            
            UE_LOG(LogTemp, Log, TEXT("Combat_ThreatDetectionSystem: New threat detected - %s at distance %.2f"), 
                *Actor->GetName(), NewThreat.Distance);
        }
    }
}

FCombat_ThreatLevel UCombat_ThreatDetectionSystem::GetThreatLevel(AActor* Actor) const
{
    if (!IsValid(Actor))
    {
        return FCombat_ThreatLevel();
    }
    
    FString ActorName = Actor->GetName();
    
    // Match actor name to threat database
    for (const FCombat_ThreatLevel& Threat : ThreatDatabase)
    {
        if (ActorName.Contains(Threat.ThreatName))
        {
            return Threat;
        }
    }
    
    // Default threat level for unknown actors
    FCombat_ThreatLevel DefaultThreat;
    DefaultThreat.ThreatName = ActorName;
    DefaultThreat.Level = EThreatLevel::Low;
    DefaultThreat.ThreatRadius = 500.0f;
    DefaultThreat.DamagePerSecond = 5.0f;
    
    return DefaultThreat;
}

bool UCombat_ThreatDetectionSystem::IsActorThreat(AActor* Actor) const
{
    if (!IsValid(Actor))
    {
        return false;
    }
    
    FString ActorName = Actor->GetName();
    
    // Check if actor name contains known threat keywords
    TArray<FString> ThreatKeywords = {
        TEXT("TRex"), TEXT("T-Rex"), TEXT("Tyrannosaurus"),
        TEXT("Raptor"), TEXT("Velociraptor"),
        TEXT("Triceratops"), TEXT("Ankylosaurus"),
        TEXT("Brachiosaurus"), TEXT("Parasaurolophus"),
        TEXT("Pachycephalo"), TEXT("Protoceratops"),
        TEXT("Tsintaosaurus"), TEXT("Combat")
    };
    
    for (const FString& Keyword : ThreatKeywords)
    {
        if (ActorName.Contains(Keyword))
        {
            return true;
        }
    }
    
    return false;
}

FVector UCombat_ThreatDetectionSystem::GetNearestThreatDirection() const
{
    if (ActiveThreats.Num() == 0)
    {
        return FVector::ZeroVector;
    }
    
    float NearestDistance = FLT_MAX;
    FVector NearestDirection = FVector::ZeroVector;
    
    for (const FCombat_DetectionData& Threat : ActiveThreats)
    {
        if (Threat.Distance < NearestDistance)
        {
            NearestDistance = Threat.Distance;
            NearestDirection = Threat.Direction;
        }
    }
    
    return NearestDirection;
}

float UCombat_ThreatDetectionSystem::GetNearestThreatDistance() const
{
    if (ActiveThreats.Num() == 0)
    {
        return -1.0f;
    }
    
    float NearestDistance = FLT_MAX;
    
    for (const FCombat_DetectionData& Threat : ActiveThreats)
    {
        if (Threat.Distance < NearestDistance)
        {
            NearestDistance = Threat.Distance;
        }
    }
    
    return NearestDistance;
}

void UCombat_ThreatDetectionSystem::AddThreatType(const FCombat_ThreatLevel& NewThreat)
{
    ThreatDatabase.Add(NewThreat);
    UE_LOG(LogTemp, Log, TEXT("Combat_ThreatDetectionSystem: Added new threat type - %s"), *NewThreat.ThreatName);
}

void UCombat_ThreatDetectionSystem::ClearThreats()
{
    for (const FCombat_DetectionData& Threat : ActiveThreats)
    {
        OnThreatLost(Threat.ThreatActor);
    }
    
    ActiveThreats.Empty();
    CurrentThreatLevel = EThreatLevel::None;
    OnThreatLevelChanged(CurrentThreatLevel);
    
    UE_LOG(LogTemp, Log, TEXT("Combat_ThreatDetectionSystem: All threats cleared"));
}