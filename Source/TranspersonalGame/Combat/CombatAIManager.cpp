#include "CombatAIManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UCombatAIManager::UCombatAIManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz update rate

    // Initialize tactical parameters
    TacticalUpdateInterval = 0.5f;
    MaxEngagementRange = 2000.0f;
    MinFlankingDistance = 500.0f;
    GroupFormationRadius = 300.0f;
    MaxPackSize = 6;
    
    bEnablePackTactics = true;
    bEnableFlankingManeuvers = true;
    bEnableAmbushTactics = true;
    
    LastTacticalUpdate = 0.0f;
    ThreatDecayRate = 0.1f;
}

void UCombatAIManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeCombatAI();
}

void UCombatAIManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateTacticalState(DeltaTime);
    UpdateThreatLevels(DeltaTime);
    
    if (bEnablePackTactics && TacticalState.AlliedUnits.Num() > 1)
    {
        CoordinateGroupTactics();
    }
}

void UCombatAIManager::InitializeCombatAI()
{
    TacticalState.CurrentMode = ECombat_TacticalMode::Patrol;
    TacticalState.GroupCohesion = 1.0f;
    TacticalState.PrimaryTarget = nullptr;
    
    // Find nearby allied units of the same type
    if (AActor* Owner = GetOwner())
    {
        UClass* OwnerClass = Owner->GetClass();
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), OwnerClass, AllActors);
        
        for (AActor* Actor : AllActors)
        {
            if (Actor != Owner)
            {
                float Distance = FVector::Dist(Owner->GetActorLocation(), Actor->GetActorLocation());
                if (Distance <= GroupFormationRadius * 2.0f)
                {
                    RegisterPackMember(Actor);
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("CombatAIManager initialized for %s with %d pack members"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"), 
           TacticalState.AlliedUnits.Num());
}

void UCombatAIManager::UpdateTacticalState(float DeltaTime)
{
    LastTacticalUpdate += DeltaTime;
    
    if (LastTacticalUpdate >= TacticalUpdateInterval)
    {
        SelectTacticalMode();
        CalculateGroupCenter();
        UpdateGroupCohesion();
        UpdatePackFormation();
        
        LastTacticalUpdate = 0.0f;
    }
}

void UCombatAIManager::AssessThreat(AActor* PotentialThreat, FCombat_ThreatAssessment& OutAssessment)
{
    if (!PotentialThreat || !GetOwner())
    {
        OutAssessment = FCombat_ThreatAssessment();
        return;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector ThreatLocation = PotentialThreat->GetActorLocation();
    
    OutAssessment.Distance = FVector::Dist(OwnerLocation, ThreatLocation);
    OutAssessment.LastKnownPosition = ThreatLocation;
    OutAssessment.TimeSinceLastSeen = 0.0f;
    
    // Base threat calculation
    float BaseThreat = 1.0f;
    
    // Distance factor (closer = more threatening)
    float DistanceFactor = FMath::Clamp(1.0f - (OutAssessment.Distance / MaxEngagementRange), 0.1f, 1.0f);
    
    // Check if it's a player character (higher threat)
    if (PotentialThreat->IsA<ACharacter>())
    {
        BaseThreat = 2.0f;
    }
    
    OutAssessment.ThreatLevel = BaseThreat * DistanceFactor;
    OutAssessment.bIsHostile = OutAssessment.ThreatLevel > 0.3f;
}

void UCombatAIManager::SelectTacticalMode()
{
    ECombat_TacticalMode NewMode = ECombat_TacticalMode::Patrol;
    
    if (ThreatMap.Num() > 0)
    {
        AActor* HighestThreat = GetHighestThreat();
        if (HighestThreat)
        {
            FCombat_ThreatAssessment* ThreatData = ThreatMap.Find(HighestThreat);
            if (ThreatData && ThreatData->ThreatLevel > 0.5f)
            {
                if (ThreatData->Distance <= MaxEngagementRange * 0.3f)
                {
                    NewMode = ECombat_TacticalMode::Assault;
                }
                else if (ThreatData->Distance <= MaxEngagementRange * 0.7f)
                {
                    NewMode = ECombat_TacticalMode::Engage;
                }
                else
                {
                    NewMode = ECombat_TacticalMode::Investigate;
                }
            }
        }
    }
    
    if (NewMode != TacticalState.CurrentMode)
    {
        TacticalState.CurrentMode = NewMode;
        UE_LOG(LogTemp, Log, TEXT("CombatAI: Tactical mode changed to %d"), (int32)NewMode);
    }
}

void UCombatAIManager::CoordinateGroupTactics()
{
    if (TacticalState.AlliedUnits.Num() < 2)
        return;
        
    AActor* PrimaryTarget = GetHighestThreat();
    if (!PrimaryTarget)
        return;
        
    TacticalState.PrimaryTarget = PrimaryTarget;
    
    // Assign roles to pack members
    for (int32 i = 0; i < TacticalState.AlliedUnits.Num(); ++i)
    {
        AActor* PackMember = TacticalState.AlliedUnits[i];
        if (!PackMember)
            continue;
            
        // Simple role assignment based on position
        if (i == 0)
        {
            // Alpha - direct engagement
            ExecuteTacticalManeuver(ECombat_TacticalManeuver::DirectAssault);
        }
        else if (i % 2 == 1)
        {
            // Flanker left
            ExecuteTacticalManeuver(ECombat_TacticalManeuver::FlankLeft);
        }
        else
        {
            // Flanker right
            ExecuteTacticalManeuver(ECombat_TacticalManeuver::FlankRight);
        }
    }
}

void UCombatAIManager::ExecuteTacticalManeuver(ECombat_TacticalManeuver Maneuver)
{
    if (!CanExecuteManeuver(Maneuver))
        return;
        
    AActor* Owner = GetOwner();
    AActor* Target = TacticalState.PrimaryTarget;
    
    if (!Owner || !Target)
        return;
        
    FVector TargetLocation;
    
    switch (Maneuver)
    {
        case ECombat_TacticalManeuver::DirectAssault:
            TargetLocation = Target->GetActorLocation();
            break;
            
        case ECombat_TacticalManeuver::FlankLeft:
            TargetLocation = CalculateFlankingPosition(Target, -1);
            break;
            
        case ECombat_TacticalManeuver::FlankRight:
            TargetLocation = CalculateFlankingPosition(Target, 1);
            break;
            
        case ECombat_TacticalManeuver::Ambush:
            // Calculate ambush position behind target
            {
                FVector TargetForward = Target->GetActorForwardVector();
                TargetLocation = Target->GetActorLocation() - (TargetForward * MinFlankingDistance);
            }
            break;
            
        case ECombat_TacticalManeuver::Retreat:
            TargetLocation = Owner->GetActorLocation() + ((Owner->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal() * MinFlankingDistance);
            break;
            
        default:
            TargetLocation = Owner->GetActorLocation();
            break;
    }
    
    // Debug visualization
    if (GetWorld())
    {
        DrawDebugSphere(GetWorld(), TargetLocation, 50.0f, 8, FColor::Red, false, 1.0f);
    }
    
    UE_LOG(LogTemp, Log, TEXT("CombatAI: Executing maneuver %d to location %s"), 
           (int32)Maneuver, *TargetLocation.ToString());
}

void UCombatAIManager::RegisterPackMember(AActor* PackMember)
{
    if (PackMember && !TacticalState.AlliedUnits.Contains(PackMember))
    {
        if (TacticalState.AlliedUnits.Num() < MaxPackSize)
        {
            TacticalState.AlliedUnits.Add(PackMember);
            UE_LOG(LogTemp, Log, TEXT("CombatAI: Registered pack member %s"), *PackMember->GetName());
        }
    }
}

void UCombatAIManager::UnregisterPackMember(AActor* PackMember)
{
    if (PackMember)
    {
        TacticalState.AlliedUnits.Remove(PackMember);
        UE_LOG(LogTemp, Log, TEXT("CombatAI: Unregistered pack member %s"), *PackMember->GetName());
    }
}

void UCombatAIManager::UpdatePackFormation()
{
    // Remove invalid pack members
    TacticalState.AlliedUnits.RemoveAll([](AActor* Actor) {
        return !IsValid(Actor);
    });
}

FVector UCombatAIManager::CalculateFlankingPosition(AActor* Target, int32 FlankIndex)
{
    if (!Target)
        return FVector::ZeroVector;
        
    FVector TargetLocation = Target->GetActorLocation();
    FVector TargetForward = Target->GetActorForwardVector();
    FVector TargetRight = Target->GetActorRightVector();
    
    // Calculate flanking position
    FVector FlankDirection = TargetRight * FlankIndex; // -1 for left, 1 for right
    FVector FlankPosition = TargetLocation + (FlankDirection * MinFlankingDistance);
    
    return FlankPosition;
}

void UCombatAIManager::AddThreat(AActor* ThreatActor, float ThreatLevel)
{
    if (!ThreatActor)
        return;
        
    FCombat_ThreatAssessment Assessment;
    AssessThreat(ThreatActor, Assessment);
    Assessment.ThreatLevel = FMath::Max(Assessment.ThreatLevel, ThreatLevel);
    
    ThreatMap.Add(ThreatActor, Assessment);
    
    UE_LOG(LogTemp, Log, TEXT("CombatAI: Added threat %s with level %.2f"), 
           *ThreatActor->GetName(), Assessment.ThreatLevel);
}

void UCombatAIManager::RemoveThreat(AActor* ThreatActor)
{
    if (ThreatActor && ThreatMap.Contains(ThreatActor))
    {
        ThreatMap.Remove(ThreatActor);
        UE_LOG(LogTemp, Log, TEXT("CombatAI: Removed threat %s"), *ThreatActor->GetName());
    }
}

AActor* UCombatAIManager::GetHighestThreat() const
{
    AActor* HighestThreat = nullptr;
    float HighestThreatLevel = 0.0f;
    
    for (const auto& ThreatPair : ThreatMap)
    {
        if (ThreatPair.Value.ThreatLevel > HighestThreatLevel)
        {
            HighestThreatLevel = ThreatPair.Value.ThreatLevel;
            HighestThreat = ThreatPair.Key;
        }
    }
    
    return HighestThreat;
}

void UCombatAIManager::UpdateThreatLevels(float DeltaTime)
{
    TArray<AActor*> ThreatsToRemove;
    
    for (auto& ThreatPair : ThreatMap)
    {
        AActor* ThreatActor = ThreatPair.Key;
        FCombat_ThreatAssessment& Assessment = ThreatPair.Value;
        
        if (!IsValid(ThreatActor))
        {
            ThreatsToRemove.Add(ThreatActor);
            continue;
        }
        
        // Update threat assessment
        FCombat_ThreatAssessment NewAssessment;
        AssessThreat(ThreatActor, NewAssessment);
        
        // Decay threat over time if not in line of sight
        Assessment.TimeSinceLastSeen += DeltaTime;
        if (Assessment.TimeSinceLastSeen > 5.0f)
        {
            Assessment.ThreatLevel = FMath::Max(0.0f, Assessment.ThreatLevel - (ThreatDecayRate * DeltaTime));
        }
        else
        {
            Assessment = NewAssessment;
        }
        
        // Remove very low threats
        if (Assessment.ThreatLevel < 0.1f)
        {
            ThreatsToRemove.Add(ThreatActor);
        }
    }
    
    // Clean up expired threats
    for (AActor* ThreatToRemove : ThreatsToRemove)
    {
        RemoveThreat(ThreatToRemove);
    }
}

bool UCombatAIManager::IsInCombat() const
{
    return TacticalState.CurrentMode == ECombat_TacticalMode::Engage || 
           TacticalState.CurrentMode == ECombat_TacticalMode::Assault;
}

ECombat_TacticalMode UCombatAIManager::GetCurrentTacticalMode() const
{
    return TacticalState.CurrentMode;
}

float UCombatAIManager::GetGroupCohesion() const
{
    return TacticalState.GroupCohesion;
}

FVector UCombatAIManager::GetOptimalAttackPosition(AActor* Target) const
{
    if (!Target)
        return FVector::ZeroVector;
        
    ECombat_TacticalManeuver OptimalManeuver = SelectOptimalManeuver(Target);
    
    switch (OptimalManeuver)
    {
        case ECombat_TacticalManeuver::FlankLeft:
            return CalculateFlankingPosition(Target, -1);
        case ECombat_TacticalManeuver::FlankRight:
            return CalculateFlankingPosition(Target, 1);
        default:
            return Target->GetActorLocation();
    }
}

void UCombatAIManager::CalculateGroupCenter()
{
    if (TacticalState.AlliedUnits.Num() == 0)
    {
        if (GetOwner())
        {
            TacticalState.GroupCenter = GetOwner()->GetActorLocation();
        }
        return;
    }
    
    FVector CenterSum = FVector::ZeroVector;
    int32 ValidUnits = 0;
    
    for (AActor* Unit : TacticalState.AlliedUnits)
    {
        if (IsValid(Unit))
        {
            CenterSum += Unit->GetActorLocation();
            ValidUnits++;
        }
    }
    
    if (ValidUnits > 0)
    {
        TacticalState.GroupCenter = CenterSum / ValidUnits;
    }
}

void UCombatAIManager::UpdateGroupCohesion()
{
    if (TacticalState.AlliedUnits.Num() <= 1)
    {
        TacticalState.GroupCohesion = 1.0f;
        return;
    }
    
    float TotalDistance = 0.0f;
    int32 ValidPairs = 0;
    
    for (int32 i = 0; i < TacticalState.AlliedUnits.Num(); ++i)
    {
        for (int32 j = i + 1; j < TacticalState.AlliedUnits.Num(); ++j)
        {
            AActor* Unit1 = TacticalState.AlliedUnits[i];
            AActor* Unit2 = TacticalState.AlliedUnits[j];
            
            if (IsValid(Unit1) && IsValid(Unit2))
            {
                float Distance = FVector::Dist(Unit1->GetActorLocation(), Unit2->GetActorLocation());
                TotalDistance += Distance;
                ValidPairs++;
            }
        }
    }
    
    if (ValidPairs > 0)
    {
        float AverageDistance = TotalDistance / ValidPairs;
        TacticalState.GroupCohesion = FMath::Clamp(1.0f - (AverageDistance / (GroupFormationRadius * 2.0f)), 0.0f, 1.0f);
    }
}

ECombat_TacticalManeuver UCombatAIManager::SelectOptimalManeuver(AActor* Target) const
{
    if (!Target || !GetOwner())
        return ECombat_TacticalManeuver::DirectAssault;
        
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector TargetLocation = Target->GetActorLocation();
    float Distance = FVector::Dist(OwnerLocation, TargetLocation);
    
    // Close range - direct assault
    if (Distance <= MinFlankingDistance * 0.5f)
    {
        return ECombat_TacticalManeuver::DirectAssault;
    }
    
    // Medium range - flanking
    if (Distance <= MaxEngagementRange * 0.7f && bEnableFlankingManeuvers)
    {
        // Randomly choose left or right flank
        return FMath::RandBool() ? ECombat_TacticalManeuver::FlankLeft : ECombat_TacticalManeuver::FlankRight;
    }
    
    // Long range - ambush if possible
    if (bEnableAmbushTactics)
    {
        return ECombat_TacticalManeuver::Ambush;
    }
    
    return ECombat_TacticalManeuver::DirectAssault;
}

bool UCombatAIManager::CanExecuteManeuver(ECombat_TacticalManeuver Maneuver) const
{
    switch (Maneuver)
    {
        case ECombat_TacticalManeuver::FlankLeft:
        case ECombat_TacticalManeuver::FlankRight:
            return bEnableFlankingManeuvers;
        case ECombat_TacticalManeuver::Ambush:
            return bEnableAmbushTactics;
        default:
            return true;
    }
}