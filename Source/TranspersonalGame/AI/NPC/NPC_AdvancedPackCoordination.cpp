#include "NPC_AdvancedPackCoordination.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UNPC_AdvancedPackCoordination::UNPC_AdvancedPackCoordination()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    AlphaPawn = nullptr;
    MaxFormationDistance = 1000.0f;
    FormationUpdateRate = 0.5f;
    CommunicationRange = 2000.0f;
    bAutoReform = true;
    
    LastFormationUpdate = 0.0f;
    LastCommunicationUpdate = 0.0f;
    
    CoordinationData.FormationRadius = 500.0f;
    CoordinationData.MovementSpeed = 300.0f;
}

void UNPC_AdvancedPackCoordination::BeginPlay()
{
    Super::BeginPlay();
    
    if (GetOwner())
    {
        if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
        {
            InitializePack(OwnerPawn);
        }
    }
}

void UNPC_AdvancedPackCoordination::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!IsPackActive())
    {
        return;
    }
    
    UpdatePackBehavior(DeltaTime);
    
    // Update formation positions
    LastFormationUpdate += DeltaTime;
    if (LastFormationUpdate >= FormationUpdateRate)
    {
        UpdateFormationPositions();
        LastFormationUpdate = 0.0f;
    }
    
    // Handle pack communication
    LastCommunicationUpdate += DeltaTime;
    if (LastCommunicationUpdate >= 1.0f)
    {
        HandlePackCommunication();
        LastCommunicationUpdate = 0.0f;
    }
}

void UNPC_AdvancedPackCoordination::InitializePack(APawn* AlphaPawn)
{
    if (!AlphaPawn)
    {
        return;
    }
    
    this->AlphaPawn = AlphaPawn;
    PackMembers.Empty();
    
    // Add alpha as first pack member
    FNPC_PackMember AlphaMember;
    AlphaMember.MemberPawn = AlphaPawn;
    AlphaMember.Role = ENPC_PackRole::Alpha;
    AlphaMember.RelativePosition = FVector::ZeroVector;
    AlphaMember.DistanceFromAlpha = 0.0f;
    AlphaMember.Stamina = 100.0f;
    AlphaMember.Aggression = 80.0f;
    
    PackMembers.Add(AlphaMember);
    
    UE_LOG(LogTemp, Warning, TEXT("Pack initialized with Alpha: %s"), *AlphaPawn->GetName());
}

void UNPC_AdvancedPackCoordination::AddPackMember(APawn* NewMember, ENPC_PackRole Role)
{
    if (!NewMember || !IsPackActive())
    {
        return;
    }
    
    // Check if member already exists
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (Member.MemberPawn == NewMember)
        {
            return;
        }
    }
    
    FNPC_PackMember NewPackMember;
    NewPackMember.MemberPawn = NewMember;
    NewPackMember.Role = Role;
    NewPackMember.RelativePosition = FVector(FMath::RandRange(-200.0f, 200.0f), FMath::RandRange(-200.0f, 200.0f), 0.0f);
    NewPackMember.DistanceFromAlpha = NewPackMember.RelativePosition.Size();
    NewPackMember.Stamina = FMath::RandRange(70.0f, 100.0f);
    NewPackMember.Aggression = FMath::RandRange(30.0f, 70.0f);
    
    PackMembers.Add(NewPackMember);
    
    UE_LOG(LogTemp, Warning, TEXT("Added pack member: %s with role: %d"), *NewMember->GetName(), (int32)Role);
}

void UNPC_AdvancedPackCoordination::RemovePackMember(APawn* Member)
{
    if (!Member)
    {
        return;
    }
    
    for (int32 i = PackMembers.Num() - 1; i >= 0; i--)
    {
        if (PackMembers[i].MemberPawn == Member)
        {
            PackMembers.RemoveAt(i);
            UE_LOG(LogTemp, Warning, TEXT("Removed pack member: %s"), *Member->GetName());
            break;
        }
    }
    
    // If alpha was removed, reassign
    if (Member == AlphaPawn && PackMembers.Num() > 0)
    {
        for (FNPC_PackMember& PackMember : PackMembers)
        {
            if (PackMember.Role == ENPC_PackRole::Beta)
            {
                AlphaPawn = PackMember.MemberPawn;
                PackMember.Role = ENPC_PackRole::Alpha;
                break;
            }
        }
    }
}

void UNPC_AdvancedPackCoordination::SetPackFormation(ENPC_PackFormation NewFormation)
{
    if (CoordinationData.CurrentFormation != NewFormation)
    {
        CoordinationData.CurrentFormation = NewFormation;
        UpdateFormationPositions();
        
        UE_LOG(LogTemp, Warning, TEXT("Pack formation changed to: %d"), (int32)NewFormation);
    }
}

void UNPC_AdvancedPackCoordination::MoveToLocation(FVector TargetLocation)
{
    CoordinationData.TargetLocation = TargetLocation;
    CoordinationData.TargetActor = nullptr;
    CoordinationData.bIsHunting = false;
    CoordinationData.bIsDefending = false;
    
    if (CoordinationData.CurrentFormation == ENPC_PackFormation::None)
    {
        SetPackFormation(ENPC_PackFormation::Line);
    }
    
    UpdateFormationPositions();
}

void UNPC_AdvancedPackCoordination::AttackTarget(AActor* Target)
{
    if (!Target)
    {
        return;
    }
    
    CoordinationData.TargetActor = Target;
    CoordinationData.TargetLocation = Target->GetActorLocation();
    CoordinationData.bIsHunting = true;
    CoordinationData.bIsDefending = false;
    
    SetPackFormation(ENPC_PackFormation::Hunting);
    
    UE_LOG(LogTemp, Warning, TEXT("Pack attacking target: %s"), *Target->GetName());
}

void UNPC_AdvancedPackCoordination::DefendPosition(FVector DefensePosition)
{
    CoordinationData.TargetLocation = DefensePosition;
    CoordinationData.TargetActor = nullptr;
    CoordinationData.bIsHunting = false;
    CoordinationData.bIsDefending = true;
    
    SetPackFormation(ENPC_PackFormation::Defensive);
}

void UNPC_AdvancedPackCoordination::ScatterFormation()
{
    SetPackFormation(ENPC_PackFormation::None);
    
    // Assign random positions around alpha
    for (FNPC_PackMember& Member : PackMembers)
    {
        if (Member.Role != ENPC_PackRole::Alpha)
        {
            float RandomAngle = FMath::RandRange(0.0f, 360.0f);
            float RandomDistance = FMath::RandRange(300.0f, 800.0f);
            
            Member.RelativePosition = FVector(
                FMath::Cos(FMath::DegreesToRadians(RandomAngle)) * RandomDistance,
                FMath::Sin(FMath::DegreesToRadians(RandomAngle)) * RandomDistance,
                0.0f
            );
        }
    }
}

void UNPC_AdvancedPackCoordination::ReformPack()
{
    if (PackMembers.Num() > 1)
    {
        SetPackFormation(ENPC_PackFormation::Line);
    }
}

void UNPC_AdvancedPackCoordination::UpdateFormationPositions()
{
    if (!IsPackActive())
    {
        return;
    }
    
    switch (CoordinationData.CurrentFormation)
    {
        case ENPC_PackFormation::Line:
            CalculateLineFormation();
            break;
        case ENPC_PackFormation::Wedge:
            CalculateWedgeFormation();
            break;
        case ENPC_PackFormation::Circle:
            CalculateCircleFormation();
            break;
        case ENPC_PackFormation::Ambush:
            CalculateAmbushFormation();
            break;
        case ENPC_PackFormation::Hunting:
            CalculateHuntingFormation();
            break;
        case ENPC_PackFormation::Defensive:
            CalculateDefensiveFormation();
            break;
        default:
            break;
    }
}

void UNPC_AdvancedPackCoordination::CalculateLineFormation()
{
    if (PackMembers.Num() <= 1)
    {
        return;
    }
    
    float Spacing = 200.0f;
    int32 MemberIndex = 0;
    
    for (FNPC_PackMember& Member : PackMembers)
    {
        if (Member.Role == ENPC_PackRole::Alpha)
        {
            Member.RelativePosition = FVector::ZeroVector;
        }
        else
        {
            Member.RelativePosition = FVector(-Spacing * MemberIndex, 0.0f, 0.0f);
            MemberIndex++;
        }
        
        Member.DistanceFromAlpha = Member.RelativePosition.Size();
    }
}

void UNPC_AdvancedPackCoordination::CalculateWedgeFormation()
{
    if (PackMembers.Num() <= 1)
    {
        return;
    }
    
    float Spacing = 150.0f;
    int32 LeftSide = 0;
    int32 RightSide = 0;
    
    for (FNPC_PackMember& Member : PackMembers)
    {
        if (Member.Role == ENPC_PackRole::Alpha)
        {
            Member.RelativePosition = FVector::ZeroVector;
        }
        else
        {
            if (LeftSide <= RightSide)
            {
                LeftSide++;
                Member.RelativePosition = FVector(-Spacing * LeftSide, -Spacing * LeftSide, 0.0f);
            }
            else
            {
                RightSide++;
                Member.RelativePosition = FVector(-Spacing * RightSide, Spacing * RightSide, 0.0f);
            }
        }
        
        Member.DistanceFromAlpha = Member.RelativePosition.Size();
    }
}

void UNPC_AdvancedPackCoordination::CalculateCircleFormation()
{
    if (PackMembers.Num() <= 1)
    {
        return;
    }
    
    float Radius = CoordinationData.FormationRadius;
    int32 NonAlphaMembers = PackMembers.Num() - 1;
    float AngleStep = 360.0f / FMath::Max(NonAlphaMembers, 1);
    int32 AngleIndex = 0;
    
    for (FNPC_PackMember& Member : PackMembers)
    {
        if (Member.Role == ENPC_PackRole::Alpha)
        {
            Member.RelativePosition = FVector::ZeroVector;
        }
        else
        {
            float Angle = AngleStep * AngleIndex;
            Member.RelativePosition = FVector(
                FMath::Cos(FMath::DegreesToRadians(Angle)) * Radius,
                FMath::Sin(FMath::DegreesToRadians(Angle)) * Radius,
                0.0f
            );
            AngleIndex++;
        }
        
        Member.DistanceFromAlpha = Member.RelativePosition.Size();
    }
}

void UNPC_AdvancedPackCoordination::CalculateAmbushFormation()
{
    // Spread members in a wide arc for ambush
    CalculateWedgeFormation();
    
    // Increase spacing for ambush
    for (FNPC_PackMember& Member : PackMembers)
    {
        if (Member.Role != ENPC_PackRole::Alpha)
        {
            Member.RelativePosition *= 2.0f;
            Member.DistanceFromAlpha = Member.RelativePosition.Size();
        }
    }
}

void UNPC_AdvancedPackCoordination::CalculateHuntingFormation()
{
    // Similar to wedge but more aggressive positioning
    CalculateWedgeFormation();
    
    // Scouts move ahead
    for (FNPC_PackMember& Member : PackMembers)
    {
        if (Member.Role == ENPC_PackRole::Scout)
        {
            Member.RelativePosition += FVector(300.0f, 0.0f, 0.0f);
            Member.DistanceFromAlpha = Member.RelativePosition.Size();
        }
    }
}

void UNPC_AdvancedPackCoordination::CalculateDefensiveFormation()
{
    // Tight circle formation for defense
    CalculateCircleFormation();
    
    // Reduce radius for tighter formation
    for (FNPC_PackMember& Member : PackMembers)
    {
        if (Member.Role != ENPC_PackRole::Alpha)
        {
            Member.RelativePosition *= 0.5f;
            Member.DistanceFromAlpha = Member.RelativePosition.Size();
        }
    }
}

void UNPC_AdvancedPackCoordination::UpdatePackBehavior(float DeltaTime)
{
    if (!IsPackActive())
    {
        return;
    }
    
    // Monitor pack health and stamina
    MonitorPackHealth();
    
    // Auto-reform if members are too far
    if (bAutoReform)
    {
        bool bNeedsReform = false;
        for (const FNPC_PackMember& Member : PackMembers)
        {
            if (Member.MemberPawn && AlphaPawn)
            {
                float Distance = FVector::Dist(Member.MemberPawn->GetActorLocation(), AlphaPawn->GetActorLocation());
                if (Distance > MaxFormationDistance)
                {
                    bNeedsReform = true;
                    break;
                }
            }
        }
        
        if (bNeedsReform && CoordinationData.CurrentFormation != ENPC_PackFormation::None)
        {
            UpdateFormationPositions();
        }
    }
}

void UNPC_AdvancedPackCoordination::HandlePackCommunication()
{
    if (!IsPackActive())
    {
        return;
    }
    
    // Simple communication system - could be expanded with audio cues
    for (FNPC_PackMember& Member : PackMembers)
    {
        if (Member.MemberPawn && AlphaPawn)
        {
            float Distance = FVector::Dist(Member.MemberPawn->GetActorLocation(), AlphaPawn->GetActorLocation());
            
            // Members too far lose coordination
            if (Distance > CommunicationRange)
            {
                Member.Aggression = FMath::Max(Member.Aggression - 5.0f, 0.0f);
            }
            else
            {
                Member.Aggression = FMath::Min(Member.Aggression + 2.0f, 100.0f);
            }
        }
    }
}

void UNPC_AdvancedPackCoordination::AssignPackRoles()
{
    if (PackMembers.Num() <= 1)
    {
        return;
    }
    
    // Auto-assign roles based on pack size and member characteristics
    int32 ScoutCount = 0;
    int32 HunterCount = 0;
    int32 DefenderCount = 0;
    
    for (FNPC_PackMember& Member : PackMembers)
    {
        if (Member.Role == ENPC_PackRole::Alpha)
        {
            continue;
        }
        
        // Assign roles based on aggression and stamina
        if (Member.Stamina > 80.0f && ScoutCount < 2)
        {
            Member.Role = ENPC_PackRole::Scout;
            ScoutCount++;
        }
        else if (Member.Aggression > 60.0f && HunterCount < 3)
        {
            Member.Role = ENPC_PackRole::Hunter;
            HunterCount++;
        }
        else if (DefenderCount < 2)
        {
            Member.Role = ENPC_PackRole::Defender;
            DefenderCount++;
        }
        else
        {
            Member.Role = ENPC_PackRole::Beta;
        }
    }
}

void UNPC_AdvancedPackCoordination::MonitorPackHealth()
{
    if (!IsPackActive())
    {
        return;
    }
    
    for (FNPC_PackMember& Member : PackMembers)
    {
        if (Member.MemberPawn)
        {
            // Simple health monitoring - could be expanded with actual health components
            Member.Stamina = FMath::Max(Member.Stamina - 0.1f, 0.0f);
            
            // Injured members get different behavior
            if (Member.Stamina < 30.0f)
            {
                Member.bIsInjured = true;
                Member.Aggression = FMath::Max(Member.Aggression - 1.0f, 0.0f);
            }
            else
            {
                Member.bIsInjured = false;
            }
        }
    }
}