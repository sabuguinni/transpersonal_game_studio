#include "NPC_PackBehaviorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"

UNPC_PackBehaviorComponent::UNPC_PackBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize default values
    MyPackRole = ENPC_PackRole::Lone;
    CurrentPackState = ENPC_PackState::Idle;
    MaxPackDistance = 2000.0f;
    HuntingRange = 3000.0f;
    FlankingRadius = 1500.0f;
    MaxPackSize = 6;
    bCanFormPacks = true;
    TimeSinceLastHunt = 0.0f;
    TimeSinceLastSignal = 0.0f;
}

void UNPC_PackBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-assign pack role based on proximity to other pack members
    if (bCanFormPacks)
    {
        // Look for nearby pack members
        UWorld* World = GetWorld();
        if (World)
        {
            TArray<AActor*> FoundActors;
            UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundActors);
            
            for (AActor* Actor : FoundActors)
            {
                if (APawn* NearbyPawn = Cast<APawn>(Actor))
                {
                    if (NearbyPawn != GetOwner())
                    {
                        UNPC_PackBehaviorComponent* OtherPackComp = NearbyPawn->FindComponentByClass<UNPC_PackBehaviorComponent>();
                        if (OtherPackComp && OtherPackComp->bCanFormPacks)
                        {
                            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), NearbyPawn->GetActorLocation());
                            if (Distance <= MaxPackDistance)
                            {
                                // Join existing pack or form new one
                                if (OtherPackComp->IsInPack())
                                {
                                    JoinPack(Cast<APawn>(GetOwner()), ENPC_PackRole::Beta);
                                }
                                else if (!IsInPack())
                                {
                                    // Form new pack with this pawn as leader
                                    MyPackRole = ENPC_PackRole::Leader;
                                    PackLeader = Cast<APawn>(GetOwner());
                                    OtherPackComp->JoinPack(Cast<APawn>(GetOwner()), ENPC_PackRole::Alpha);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void UNPC_PackBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TimeSinceLastHunt += DeltaTime;
    TimeSinceLastSignal += DeltaTime;

    UpdatePackFormation();
    UpdateHuntingBehavior();
    CheckPackCohesion();
}

void UNPC_PackBehaviorComponent::JoinPack(APawn* NewMember, ENPC_PackRole Role)
{
    if (!NewMember || PackMembers.Num() >= MaxPackSize)
    {
        return;
    }

    // Check if already in pack
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (Member.PackMember.Get() == NewMember)
        {
            return; // Already in pack
        }
    }

    FNPC_PackMember NewPackMember;
    NewPackMember.PackMember = NewMember;
    NewPackMember.Role = Role;
    NewPackMember.bIsAlive = true;
    NewPackMember.DistanceFromLeader = 0.0f;

    PackMembers.Add(NewPackMember);

    // Update the new member's pack component
    if (UNPC_PackBehaviorComponent* MemberPackComp = NewMember->FindComponentByClass<UNPC_PackBehaviorComponent>())
    {
        MemberPackComp->MyPackRole = Role;
        MemberPackComp->PackLeader = PackLeader;
        MemberPackComp->CurrentPackState = CurrentPackState;
    }

    UE_LOG(LogTemp, Log, TEXT("Pack member joined: %s with role %d"), *NewMember->GetName(), (int32)Role);
}

void UNPC_PackBehaviorComponent::LeavePack(APawn* Member)
{
    if (!Member)
    {
        return;
    }

    for (int32 i = PackMembers.Num() - 1; i >= 0; i--)
    {
        if (PackMembers[i].PackMember.Get() == Member)
        {
            PackMembers.RemoveAt(i);
            
            // Update the leaving member's pack component
            if (UNPC_PackBehaviorComponent* MemberPackComp = Member->FindComponentByClass<UNPC_PackBehaviorComponent>())
            {
                MemberPackComp->MyPackRole = ENPC_PackRole::Lone;
                MemberPackComp->PackLeader = nullptr;
                MemberPackComp->PackMembers.Empty();
            }
            
            UE_LOG(LogTemp, Log, TEXT("Pack member left: %s"), *Member->GetName());
            break;
        }
    }
}

void UNPC_PackBehaviorComponent::SetPackState(ENPC_PackState NewState)
{
    if (CurrentPackState != NewState)
    {
        HandlePackStateTransition(NewState);
        CurrentPackState = NewState;
        
        // Propagate state to all pack members
        for (const FNPC_PackMember& Member : PackMembers)
        {
            if (APawn* MemberPawn = Member.PackMember.Get())
            {
                if (UNPC_PackBehaviorComponent* MemberPackComp = MemberPawn->FindComponentByClass<UNPC_PackBehaviorComponent>())
                {
                    MemberPackComp->CurrentPackState = NewState;
                }
            }
        }
    }
}

APawn* UNPC_PackBehaviorComponent::GetPackLeader() const
{
    return PackLeader.Get();
}

TArray<APawn*> UNPC_PackBehaviorComponent::GetPackMembers() const
{
    TArray<APawn*> Members;
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (APawn* MemberPawn = Member.PackMember.Get())
        {
            Members.Add(MemberPawn);
        }
    }
    return Members;
}

bool UNPC_PackBehaviorComponent::IsInPack() const
{
    return PackMembers.Num() > 0 || PackLeader.IsValid();
}

void UNPC_PackBehaviorComponent::InitiatePackHunt(APawn* Target)
{
    if (!Target || !IsInPack())
    {
        return;
    }

    CurrentTarget = Target;
    SetPackState(ENPC_PackState::Hunting);
    TimeSinceLastHunt = 0.0f;

    // Send hunt signal to all pack members
    SendPackSignal(TEXT("InitiateHunt"));

    UE_LOG(LogTemp, Log, TEXT("Pack hunt initiated against target: %s"), *Target->GetName());
}

FVector UNPC_PackBehaviorComponent::GetFlankingPosition(APawn* Target, int32 MemberIndex)
{
    if (!Target || !PackLeader.IsValid())
    {
        return FVector::ZeroVector;
    }

    FVector TargetLocation = Target->GetActorLocation();
    FVector LeaderLocation = PackLeader->GetActorLocation();
    
    // Calculate flanking positions in a circle around the target
    float AngleStep = 360.0f / FMath::Max(PackMembers.Num(), 1);
    float MemberAngle = AngleStep * MemberIndex;
    
    FVector FlankDirection = FVector(
        FMath::Cos(FMath::DegreesToRadians(MemberAngle)),
        FMath::Sin(FMath::DegreesToRadians(MemberAngle)),
        0.0f
    );

    return TargetLocation + (FlankDirection * FlankingRadius);
}

void UNPC_PackBehaviorComponent::CoordinateAttack(APawn* Target)
{
    if (!Target || MyPackRole != ENPC_PackRole::Leader)
    {
        return;
    }

    SetPackState(ENPC_PackState::Attacking);
    SendPackSignal(TEXT("CoordinatedAttack"));

    UE_LOG(LogTemp, Log, TEXT("Coordinated attack initiated by pack leader"));
}

void UNPC_PackBehaviorComponent::SendPackSignal(const FString& SignalType)
{
    TimeSinceLastSignal = 0.0f;
    
    // Send signal to all pack members
    for (const FNPC_PackMember& Member : PackMembers)
    {
        if (APawn* MemberPawn = Member.PackMember.Get())
        {
            if (UNPC_PackBehaviorComponent* MemberPackComp = MemberPawn->FindComponentByClass<UNPC_PackBehaviorComponent>())
            {
                MemberPackComp->ReceivePackSignal(SignalType, Cast<APawn>(GetOwner()));
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Pack signal sent: %s"), *SignalType);
}

void UNPC_PackBehaviorComponent::ReceivePackSignal(const FString& SignalType, APawn* Sender)
{
    if (!Sender)
    {
        return;
    }

    if (SignalType == TEXT("InitiateHunt"))
    {
        SetPackState(ENPC_PackState::Hunting);
    }
    else if (SignalType == TEXT("CoordinatedAttack"))
    {
        SetPackState(ENPC_PackState::Attacking);
    }
    else if (SignalType == TEXT("Retreat"))
    {
        SetPackState(ENPC_PackState::Fleeing);
    }

    UE_LOG(LogTemp, Log, TEXT("Pack signal received: %s from %s"), *SignalType, *Sender->GetName());
}

void UNPC_PackBehaviorComponent::UpdatePackFormation()
{
    if (!IsInPack() || !PackLeader.IsValid())
    {
        return;
    }

    // Update distances from leader for all pack members
    FVector LeaderLocation = PackLeader->GetActorLocation();
    
    for (FNPC_PackMember& Member : PackMembers)
    {
        if (APawn* MemberPawn = Member.PackMember.Get())
        {
            Member.DistanceFromLeader = FVector::Dist(LeaderLocation, MemberPawn->GetActorLocation());
        }
    }
}

void UNPC_PackBehaviorComponent::UpdateHuntingBehavior()
{
    if (CurrentPackState != ENPC_PackState::Hunting || !CurrentTarget.IsValid())
    {
        return;
    }

    // Check if target is still within hunting range
    float DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
    
    if (DistanceToTarget > HuntingRange)
    {
        // Target escaped, return to idle
        SetPackState(ENPC_PackState::Idle);
        CurrentTarget = nullptr;
    }
    else if (DistanceToTarget < 500.0f && MyPackRole == ENPC_PackRole::Leader)
    {
        // Close enough to initiate coordinated attack
        CoordinateAttack(CurrentTarget.Get());
    }
}

void UNPC_PackBehaviorComponent::CheckPackCohesion()
{
    if (!IsInPack())
    {
        return;
    }

    // Remove dead or missing pack members
    for (int32 i = PackMembers.Num() - 1; i >= 0; i--)
    {
        if (!PackMembers[i].PackMember.IsValid() || !PackMembers[i].bIsAlive)
        {
            PackMembers.RemoveAt(i);
        }
        else if (PackMembers[i].DistanceFromLeader > MaxPackDistance * 2.0f)
        {
            // Member is too far, remove from pack
            LeavePack(PackMembers[i].PackMember.Get());
        }
    }

    // If pack is too small, disband
    if (PackMembers.Num() < 2 && MyPackRole == ENPC_PackRole::Leader)
    {
        for (const FNPC_PackMember& Member : PackMembers)
        {
            LeavePack(Member.PackMember.Get());
        }
        MyPackRole = ENPC_PackRole::Lone;
        PackLeader = nullptr;
    }
}

FVector UNPC_PackBehaviorComponent::CalculateFormationPosition()
{
    if (!PackLeader.IsValid() || MyPackRole == ENPC_PackRole::Leader)
    {
        return GetOwner()->GetActorLocation();
    }

    FVector LeaderLocation = PackLeader->GetActorLocation();
    FVector LeaderForward = PackLeader->GetActorForwardVector();
    
    // Calculate position based on pack role
    float OffsetDistance = 300.0f;
    FVector Offset = FVector::ZeroVector;
    
    switch (MyPackRole)
    {
        case ENPC_PackRole::Alpha:
            Offset = LeaderForward.RotateAngleAxis(-45.0f, FVector::UpVector) * OffsetDistance;
            break;
        case ENPC_PackRole::Beta:
            Offset = LeaderForward.RotateAngleAxis(45.0f, FVector::UpVector) * OffsetDistance;
            break;
        case ENPC_PackRole::Omega:
            Offset = -LeaderForward * OffsetDistance;
            break;
        default:
            Offset = FVector::ZeroVector;
            break;
    }

    return LeaderLocation + Offset;
}

void UNPC_PackBehaviorComponent::HandlePackStateTransition(ENPC_PackState NewState)
{
    switch (NewState)
    {
        case ENPC_PackState::Hunting:
            UE_LOG(LogTemp, Log, TEXT("Pack entering hunting state"));
            break;
        case ENPC_PackState::Attacking:
            UE_LOG(LogTemp, Log, TEXT("Pack entering attack state"));
            break;
        case ENPC_PackState::Fleeing:
            UE_LOG(LogTemp, Log, TEXT("Pack entering flee state"));
            CurrentTarget = nullptr;
            break;
        case ENPC_PackState::Idle:
            UE_LOG(LogTemp, Log, TEXT("Pack returning to idle state"));
            CurrentTarget = nullptr;
            break;
        default:
            break;
    }
}