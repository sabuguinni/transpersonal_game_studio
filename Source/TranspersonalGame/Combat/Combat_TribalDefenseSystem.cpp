#include "Combat_TribalDefenseSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"

UCombat_TribalDefenseSystem::UCombat_TribalDefenseSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f;

    // Initialize default values
    CurrentFormation = ECombat_DefenseFormation::Circle;
    FormationRadius = 500.0f;
    MaxDefenders = 8;
    CurrentThreatLevel = ECombat_ThreatLevel::None;
    ThreatDetectionRadius = 1500.0f;
    ThreatUpdateInterval = 2.0f;
    AlertRadius = 2000.0f;
    bUseVisualSignals = true;
    bUseAudioSignals = true;
    
    // Internal state
    LastThreatUpdate = 0.0f;
    bInCombat = false;
    LastKnownThreatLocation = FVector::ZeroVector;

    // Initialize threat responses
    ThreatResponses.SetNum(5);
    
    // No threat response
    ThreatResponses[0].ThreatLevel = ECombat_ThreatLevel::None;
    ThreatResponses[0].PreferredFormation = ECombat_DefenseFormation::Scatter;
    ThreatResponses[0].ResponseTime = 0.0f;
    ThreatResponses[0].MinDefenders = 0;
    ThreatResponses[0].bRequiresEvacuation = false;

    // Low threat response
    ThreatResponses[1].ThreatLevel = ECombat_ThreatLevel::Low;
    ThreatResponses[1].PreferredFormation = ECombat_DefenseFormation::Line;
    ThreatResponses[1].ResponseTime = 3.0f;
    ThreatResponses[1].MinDefenders = 2;
    ThreatResponses[1].bRequiresEvacuation = false;

    // Medium threat response
    ThreatResponses[2].ThreatLevel = ECombat_ThreatLevel::Medium;
    ThreatResponses[2].PreferredFormation = ECombat_DefenseFormation::Circle;
    ThreatResponses[2].ResponseTime = 2.0f;
    ThreatResponses[2].MinDefenders = 4;
    ThreatResponses[2].bRequiresEvacuation = false;

    // High threat response
    ThreatResponses[3].ThreatLevel = ECombat_ThreatLevel::High;
    ThreatResponses[3].PreferredFormation = ECombat_DefenseFormation::Wedge;
    ThreatResponses[3].ResponseTime = 1.5f;
    ThreatResponses[3].MinDefenders = 6;
    ThreatResponses[3].bRequiresEvacuation = false;

    // Critical threat response
    ThreatResponses[4].ThreatLevel = ECombat_ThreatLevel::Critical;
    ThreatResponses[4].PreferredFormation = ECombat_DefenseFormation::Retreat;
    ThreatResponses[4].ResponseTime = 1.0f;
    ThreatResponses[4].MinDefenders = 8;
    ThreatResponses[4].bRequiresEvacuation = true;
}

void UCombat_TribalDefenseSystem::BeginPlay()
{
    Super::BeginPlay();
    
    GenerateDefensePositions();
    AssignDefenseRoles();
}

void UCombat_TribalDefenseSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update threat assessment periodically
    if (CurrentTime - LastThreatUpdate >= ThreatUpdateInterval)
    {
        AssessThreatLevel();
        LastThreatUpdate = CurrentTime;
    }

    // Check formation integrity
    if (bInCombat)
    {
        CheckFormationIntegrity();
    }
}

void UCombat_TribalDefenseSystem::SetDefenseFormation(ECombat_DefenseFormation NewFormation)
{
    if (CurrentFormation != NewFormation)
    {
        CurrentFormation = NewFormation;
        GenerateDefensePositions();
        SendFormationCommand(NewFormation);
        
        UE_LOG(LogTemp, Log, TEXT("Defense formation changed to: %d"), (int32)NewFormation);
    }
}

void UCombat_TribalDefenseSystem::GenerateDefensePositions()
{
    DefensePositions.Empty();
    
    FVector CenterLocation = GetOwner()->GetActorLocation();
    int32 NumPositions = FMath::Min(MaxDefenders, 12);

    switch (CurrentFormation)
    {
        case ECombat_DefenseFormation::Circle:
        {
            float AngleStep = 360.0f / NumPositions;
            for (int32 i = 0; i < NumPositions; i++)
            {
                float Angle = i * AngleStep;
                FVector Offset = FVector(
                    FMath::Cos(FMath::DegreesToRadians(Angle)) * FormationRadius,
                    FMath::Sin(FMath::DegreesToRadians(Angle)) * FormationRadius,
                    0.0f
                );
                
                FCombat_DefensePosition Position;
                Position.Position = CenterLocation + Offset;
                Position.Rotation = FRotator(0.0f, Angle + 180.0f, 0.0f);
                Position.Priority = i + 1;
                Position.bIsOccupied = false;
                
                DefensePositions.Add(Position);
            }
            break;
        }
        
        case ECombat_DefenseFormation::Line:
        {
            float LineWidth = FormationRadius * 2.0f;
            float Spacing = LineWidth / (NumPositions - 1);
            
            for (int32 i = 0; i < NumPositions; i++)
            {
                FVector Offset = FVector(
                    0.0f,
                    -LineWidth * 0.5f + i * Spacing,
                    0.0f
                );
                
                FCombat_DefensePosition Position;
                Position.Position = CenterLocation + Offset;
                Position.Rotation = FRotator(0.0f, 0.0f, 0.0f);
                Position.Priority = FMath::Abs(i - NumPositions / 2) + 1;
                Position.bIsOccupied = false;
                
                DefensePositions.Add(Position);
            }
            break;
        }
        
        case ECombat_DefenseFormation::Wedge:
        {
            int32 FrontRow = FMath::Min(3, NumPositions);
            int32 BackRows = NumPositions - FrontRow;
            
            // Front row
            for (int32 i = 0; i < FrontRow; i++)
            {
                FVector Offset = FVector(
                    FormationRadius * 0.5f,
                    (i - FrontRow * 0.5f) * 150.0f,
                    0.0f
                );
                
                FCombat_DefensePosition Position;
                Position.Position = CenterLocation + Offset;
                Position.Rotation = FRotator(0.0f, 0.0f, 0.0f);
                Position.Priority = i + 1;
                Position.bIsOccupied = false;
                
                DefensePositions.Add(Position);
            }
            
            // Back rows
            for (int32 i = 0; i < BackRows; i++)
            {
                FVector Offset = FVector(
                    -FormationRadius * 0.3f - (i / 3) * 200.0f,
                    ((i % 3) - 1) * 200.0f,
                    0.0f
                );
                
                FCombat_DefensePosition Position;
                Position.Position = CenterLocation + Offset;
                Position.Rotation = FRotator(0.0f, 0.0f, 0.0f);
                Position.Priority = FrontRow + i + 1;
                Position.bIsOccupied = false;
                
                DefensePositions.Add(Position);
            }
            break;
        }
        
        case ECombat_DefenseFormation::Scatter:
        {
            for (int32 i = 0; i < NumPositions; i++)
            {
                FVector RandomOffset = FVector(
                    FMath::RandRange(-FormationRadius, FormationRadius),
                    FMath::RandRange(-FormationRadius, FormationRadius),
                    0.0f
                );
                
                FCombat_DefensePosition Position;
                Position.Position = CenterLocation + RandomOffset;
                Position.Rotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
                Position.Priority = i + 1;
                Position.bIsOccupied = false;
                
                DefensePositions.Add(Position);
            }
            break;
        }
        
        case ECombat_DefenseFormation::Retreat:
        {
            FVector SafeDirection = GetSafeRetreatPosition() - CenterLocation;
            SafeDirection.Normalize();
            
            for (int32 i = 0; i < NumPositions; i++)
            {
                FVector Offset = SafeDirection * (FormationRadius + i * 100.0f);
                Offset += FVector(
                    FMath::RandRange(-100.0f, 100.0f),
                    FMath::RandRange(-100.0f, 100.0f),
                    0.0f
                );
                
                FCombat_DefensePosition Position;
                Position.Position = CenterLocation + Offset;
                Position.Rotation = UKismetMathLibrary::FindLookAtRotation(Position.Position, CenterLocation);
                Position.Priority = NumPositions - i;
                Position.bIsOccupied = false;
                
                DefensePositions.Add(Position);
            }
            break;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Generated %d defense positions for formation: %d"), DefensePositions.Num(), (int32)CurrentFormation);
}

FVector UCombat_TribalDefenseSystem::GetOptimalDefensePosition(AActor* Defender)
{
    if (!Defender || DefensePositions.Num() == 0)
    {
        return GetOwner()->GetActorLocation();
    }

    // Find the best available position based on priority and distance
    FCombat_DefensePosition* BestPosition = nullptr;
    float BestScore = -1.0f;
    
    for (FCombat_DefensePosition& Position : DefensePositions)
    {
        if (Position.bIsOccupied)
        {
            continue;
        }
        
        float Distance = FVector::Dist(Defender->GetActorLocation(), Position.Position);
        float Score = (1.0f / Position.Priority) * 100.0f - Distance * 0.1f;
        
        if (Score > BestScore)
        {
            BestScore = Score;
            BestPosition = &Position;
        }
    }
    
    return BestPosition ? BestPosition->Position : GetOwner()->GetActorLocation();
}

bool UCombat_TribalDefenseSystem::AssignDefensePosition(AActor* Defender, const FVector& Position)
{
    if (!Defender)
    {
        return false;
    }

    // Find the position and mark it as occupied
    for (FCombat_DefensePosition& DefensePos : DefensePositions)
    {
        if (FVector::Dist(DefensePos.Position, Position) < 50.0f && !DefensePos.bIsOccupied)
        {
            DefensePos.bIsOccupied = true;
            
            // Add to active defenders if not already present
            if (!ActiveDefenders.Contains(Defender))
            {
                ActiveDefenders.Add(Defender);
            }
            
            UE_LOG(LogTemp, Log, TEXT("Assigned defense position to %s"), *Defender->GetName());
            return true;
        }
    }
    
    return false;
}

void UCombat_TribalDefenseSystem::AssessThreatLevel()
{
    TArray<AActor*> DetectedThreats = DetectThreats();
    ECombat_ThreatLevel NewThreatLevel = CalculateThreatLevel(DetectedThreats);
    
    if (NewThreatLevel != CurrentThreatLevel)
    {
        CurrentThreatLevel = NewThreatLevel;
        RespondToThreat(NewThreatLevel);
        BroadcastAlert(NewThreatLevel);
        
        UE_LOG(LogTemp, Warning, TEXT("Threat level changed to: %d"), (int32)NewThreatLevel);
    }
}

void UCombat_TribalDefenseSystem::RespondToThreat(ECombat_ThreatLevel ThreatLevel)
{
    // Find appropriate response
    FCombat_ThreatResponse* Response = nullptr;
    for (FCombat_ThreatResponse& ThreatResponse : ThreatResponses)
    {
        if (ThreatResponse.ThreatLevel == ThreatLevel)
        {
            Response = &ThreatResponse;
            break;
        }
    }
    
    if (!Response)
    {
        return;
    }

    // Set appropriate formation
    SetDefenseFormation(Response->PreferredFormation);
    
    // Handle evacuation if required
    if (Response->bRequiresEvacuation)
    {
        HandleEmergencyEvacuation();
    }
    
    // Update combat state
    bInCombat = (ThreatLevel > ECombat_ThreatLevel::None);
    
    UE_LOG(LogTemp, Log, TEXT("Responding to threat level %d with formation %d"), 
           (int32)ThreatLevel, (int32)Response->PreferredFormation);
}

TArray<AActor*> UCombat_TribalDefenseSystem::DetectThreats()
{
    TArray<AActor*> Threats;
    
    if (!GetWorld())
    {
        return Threats;
    }

    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Get all actors in detection radius
    TArray<AActor*> OverlappingActors;
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        OwnerLocation,
        ThreatDetectionRadius,
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        nullptr,
        TArray<AActor*>(),
        OverlappingActors
    );
    
    // Filter for actual threats (dinosaurs, hostile NPCs, etc.)
    for (AActor* Actor : OverlappingActors)
    {
        if (Actor && Actor != GetOwner() && !TribalMembers.Contains(Actor))
        {
            // Check if actor has threatening characteristics
            if (Actor->GetName().Contains(TEXT("Dinosaur")) || 
                Actor->GetName().Contains(TEXT("Predator")) ||
                Actor->GetName().Contains(TEXT("TRex")) ||
                Actor->GetName().Contains(TEXT("Raptor")))
            {
                Threats.Add(Actor);
            }
        }
    }
    
    return Threats;
}

ECombat_ThreatLevel UCombat_TribalDefenseSystem::CalculateThreatLevel(const TArray<AActor*>& Threats)
{
    if (Threats.Num() == 0)
    {
        return ECombat_ThreatLevel::None;
    }
    
    int32 ThreatScore = 0;
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    for (AActor* Threat : Threats)
    {
        if (!Threat)
        {
            continue;
        }
        
        float Distance = FVector::Dist(OwnerLocation, Threat->GetActorLocation());
        int32 BaseScore = 1;
        
        // Increase score based on threat type
        FString ThreatName = Threat->GetName();
        if (ThreatName.Contains(TEXT("TRex")))
        {
            BaseScore = 5;
        }
        else if (ThreatName.Contains(TEXT("Raptor")))
        {
            BaseScore = 3;
        }
        else if (ThreatName.Contains(TEXT("Predator")))
        {
            BaseScore = 2;
        }
        
        // Increase score based on proximity
        if (Distance < 500.0f)
        {
            BaseScore *= 3;
        }
        else if (Distance < 1000.0f)
        {
            BaseScore *= 2;
        }
        
        ThreatScore += BaseScore;
    }
    
    // Determine threat level based on score
    if (ThreatScore >= 15)
    {
        return ECombat_ThreatLevel::Critical;
    }
    else if (ThreatScore >= 10)
    {
        return ECombat_ThreatLevel::High;
    }
    else if (ThreatScore >= 5)
    {
        return ECombat_ThreatLevel::Medium;
    }
    else if (ThreatScore >= 1)
    {
        return ECombat_ThreatLevel::Low;
    }
    
    return ECombat_ThreatLevel::None;
}

void UCombat_TribalDefenseSystem::BroadcastAlert(ECombat_ThreatLevel ThreatLevel)
{
    if (!GetWorld())
    {
        return;
    }

    FVector AlertLocation = GetOwner()->GetActorLocation();
    
    // Alert nearby tribal members
    for (AActor* Member : TribalMembers)
    {
        if (Member && FVector::Dist(AlertLocation, Member->GetActorLocation()) <= AlertRadius)
        {
            // In a real implementation, this would send a message to the tribal member
            UE_LOG(LogTemp, Log, TEXT("Alerting tribal member %s of threat level %d"), 
                   *Member->GetName(), (int32)ThreatLevel);
        }
    }
    
    // Alert nearby tribes
    if (ThreatLevel >= ECombat_ThreatLevel::Medium)
    {
        AlertNearbyTribes(AlertLocation);
    }
}

void UCombat_TribalDefenseSystem::SendFormationCommand(ECombat_DefenseFormation Formation)
{
    for (AActor* Defender : ActiveDefenders)
    {
        if (Defender)
        {
            FVector OptimalPosition = GetOptimalDefensePosition(Defender);
            // In a real implementation, this would send movement commands to the AI
            UE_LOG(LogTemp, Log, TEXT("Sending formation command to %s: move to %s"), 
                   *Defender->GetName(), *OptimalPosition.ToString());
        }
    }
}

void UCombat_TribalDefenseSystem::AlertNearbyTribes(const FVector& ThreatLocation)
{
    // Find other tribal defense systems in the area
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);
    
    for (AActor* Actor : NearbyActors)
    {
        if (Actor && Actor != GetOwner())
        {
            UCombat_TribalDefenseSystem* OtherDefenseSystem = Actor->FindComponentByClass<UCombat_TribalDefenseSystem>();
            if (OtherDefenseSystem && FVector::Dist(ThreatLocation, Actor->GetActorLocation()) <= AlertRadius * 2.0f)
            {
                // In a real implementation, this would trigger the other tribe's threat assessment
                UE_LOG(LogTemp, Log, TEXT("Alerting nearby tribe at %s"), *Actor->GetActorLocation().ToString());
            }
        }
    }
}

void UCombat_TribalDefenseSystem::RegisterTribalMember(AActor* Member)
{
    if (Member && !TribalMembers.Contains(Member))
    {
        TribalMembers.Add(Member);
        UE_LOG(LogTemp, Log, TEXT("Registered tribal member: %s"), *Member->GetName());
    }
}

void UCombat_TribalDefenseSystem::UnregisterTribalMember(AActor* Member)
{
    if (Member)
    {
        TribalMembers.Remove(Member);
        ActiveDefenders.Remove(Member);
        UE_LOG(LogTemp, Log, TEXT("Unregistered tribal member: %s"), *Member->GetName());
    }
}

void UCombat_TribalDefenseSystem::AssignDefenseRoles()
{
    ActiveDefenders.Empty();
    
    // Assign defense roles based on tribal member capabilities
    for (AActor* Member : TribalMembers)
    {
        if (Member && ActiveDefenders.Num() < MaxDefenders)
        {
            // In a real implementation, this would check member's combat capability
            ActiveDefenders.Add(Member);
        }
    }
    
    // Assign defense leader (strongest or most experienced member)
    if (ActiveDefenders.Num() > 0)
    {
        DefenseLeader = ActiveDefenders[0];
    }
    
    UE_LOG(LogTemp, Log, TEXT("Assigned %d defenders from %d tribal members"), 
           ActiveDefenders.Num(), TribalMembers.Num());
}

bool UCombat_TribalDefenseSystem::CanDefendAgainstThreat(ECombat_ThreatLevel ThreatLevel)
{
    FCombat_ThreatResponse* Response = nullptr;
    for (FCombat_ThreatResponse& ThreatResponse : ThreatResponses)
    {
        if (ThreatResponse.ThreatLevel == ThreatLevel)
        {
            Response = &ThreatResponse;
            break;
        }
    }
    
    if (!Response)
    {
        return false;
    }
    
    return ActiveDefenders.Num() >= Response->MinDefenders;
}

float UCombat_TribalDefenseSystem::GetDistanceToThreat(AActor* Threat)
{
    if (!Threat)
    {
        return -1.0f;
    }
    
    return FVector::Dist(GetOwner()->GetActorLocation(), Threat->GetActorLocation());
}

bool UCombat_TribalDefenseSystem::IsPositionSafe(const FVector& Position)
{
    TArray<AActor*> NearbyThreats = DetectThreats();
    
    for (AActor* Threat : NearbyThreats)
    {
        if (Threat && FVector::Dist(Position, Threat->GetActorLocation()) < 800.0f)
        {
            return false;
        }
    }
    
    return true;
}

FVector UCombat_TribalDefenseSystem::GetSafeRetreatPosition()
{
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    TArray<AActor*> Threats = DetectThreats();
    
    if (Threats.Num() == 0)
    {
        return OwnerLocation;
    }
    
    // Calculate average threat direction
    FVector ThreatDirection = FVector::ZeroVector;
    for (AActor* Threat : Threats)
    {
        if (Threat)
        {
            ThreatDirection += (Threat->GetActorLocation() - OwnerLocation).GetSafeNormal();
        }
    }
    ThreatDirection = ThreatDirection.GetSafeNormal();
    
    // Retreat in opposite direction
    FVector RetreatDirection = -ThreatDirection;
    FVector SafePosition = OwnerLocation + RetreatDirection * FormationRadius * 3.0f;
    
    return SafePosition;
}

void UCombat_TribalDefenseSystem::UpdateDefensePositions()
{
    // Clear occupation status
    for (FCombat_DefensePosition& Position : DefensePositions)
    {
        Position.bIsOccupied = false;
    }
    
    // Reassign positions to active defenders
    for (AActor* Defender : ActiveDefenders)
    {
        if (Defender)
        {
            FVector OptimalPosition = GetOptimalDefensePosition(Defender);
            AssignDefensePosition(Defender, OptimalPosition);
        }
    }
}

void UCombat_TribalDefenseSystem::CheckFormationIntegrity()
{
    int32 DefendersInPosition = 0;
    
    for (const FCombat_DefensePosition& Position : DefensePositions)
    {
        if (Position.bIsOccupied)
        {
            DefendersInPosition++;
        }
    }
    
    // If formation is broken, regenerate positions
    if (DefendersInPosition < ActiveDefenders.Num() * 0.5f)
    {
        UpdateDefensePositions();
        UE_LOG(LogTemp, Warning, TEXT("Formation integrity compromised, updating positions"));
    }
}

void UCombat_TribalDefenseSystem::HandleEmergencyEvacuation()
{
    FVector SafePosition = GetSafeRetreatPosition();
    
    // Move all tribal members to safe position
    for (AActor* Member : TribalMembers)
    {
        if (Member)
        {
            // In a real implementation, this would command the AI to move to safety
            UE_LOG(LogTemp, Warning, TEXT("Evacuating %s to safe position %s"), 
                   *Member->GetName(), *SafePosition.ToString());
        }
    }
    
    // Set retreat formation
    SetDefenseFormation(ECombat_DefenseFormation::Retreat);
}

ECombat_DefenseFormation UCombat_TribalDefenseSystem::SelectOptimalFormation(const TArray<AActor*>& Threats)
{
    if (Threats.Num() == 0)
    {
        return ECombat_DefenseFormation::Scatter;
    }
    
    int32 ThreatCount = Threats.Num();
    bool bHasLargeThreat = false;
    
    // Check for large threats
    for (AActor* Threat : Threats)
    {
        if (Threat && Threat->GetName().Contains(TEXT("TRex")))
        {
            bHasLargeThreat = true;
            break;
        }
    }
    
    // Select formation based on threat characteristics
    if (bHasLargeThreat)
    {
        return ECombat_DefenseFormation::Retreat;
    }
    else if (ThreatCount >= 4)
    {
        return ECombat_DefenseFormation::Circle;
    }
    else if (ThreatCount >= 2)
    {
        return ECombat_DefenseFormation::Wedge;
    }
    else
    {
        return ECombat_DefenseFormation::Line;
    }
}