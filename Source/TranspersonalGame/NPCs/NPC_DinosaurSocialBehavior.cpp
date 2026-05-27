#include "NPC_DinosaurSocialBehavior.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"

UNPC_DinosaurSocialBehavior::UNPC_DinosaurSocialBehavior()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Default values
    CurrentSocialState = ENPC_SocialState::Solitary;
    PackRole = ENPC_PackRole::Hunter;
    SocialRadius = 5000.0f;
    PackCohesionStrength = 0.7f;
    MaxPackSize = 6;
    TerritorialRadius = 8000.0f;
    
    PackLeader = nullptr;
    PackCenter = FVector::ZeroVector;
    
    MemoryDecayRate = 0.1f;
    MaxMemoryDuration = 300.0f;
    
    CommunicationRange = 3000.0f;
    LastCommunicationTime = 0.0f;
    CommunicationCooldown = 5.0f;
    
    bIsMatingSeason = false;
    MatingDesire = 0.0f;
    MatingTarget = nullptr;
    
    TimeSinceLastSocialUpdate = 0.0f;
    SocialUpdateInterval = 2.0f;
}

void UNPC_DinosaurSocialBehavior::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize social state
    CurrentSocialState = ENPC_SocialState::Solitary;
    PackCenter = GetOwner()->GetActorLocation();
    
    UE_LOG(LogTemp, Log, TEXT("NPC_DinosaurSocialBehavior initialized for %s"), *GetOwner()->GetName());
}

void UNPC_DinosaurSocialBehavior::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    TimeSinceLastSocialUpdate += DeltaTime;
    
    if (TimeSinceLastSocialUpdate >= SocialUpdateInterval)
    {
        UpdateSocialState();
        ProcessSocialInteractions();
        UpdateSocialMemories(DeltaTime);
        
        TimeSinceLastSocialUpdate = 0.0f;
    }
    
    // Continuous behaviors
    UpdatePackBehavior();
    HandleTerritorialBehavior();
    ProcessMatingBehavior();
}

void UNPC_DinosaurSocialBehavior::UpdateSocialState()
{
    if (!GetOwner()) return;
    
    ScanForNearbyDinosaurs();
    
    switch (CurrentSocialState)
    {
        case ENPC_SocialState::Solitary:
        {
            // Look for pack opportunities
            if (PackMembers.Num() > 0)
            {
                CurrentSocialState = ENPC_SocialState::Seeking;
            }
            else if (bIsMatingSeason && MatingDesire > 0.5f)
            {
                CurrentSocialState = ENPC_SocialState::Mating;
            }
            break;
        }
        
        case ENPC_SocialState::Seeking:
        {
            // Try to join or form a pack
            if (PackMembers.Num() >= 2)
            {
                if (!PackLeader)
                {
                    EstablishPackLeadership();
                }
                CurrentSocialState = ENPC_SocialState::InPack;
                CommunicateWithPack("PackFormed");
            }
            break;
        }
        
        case ENPC_SocialState::InPack:
        {
            // Maintain pack cohesion
            if (PackMembers.Num() < 2)
            {
                CurrentSocialState = ENPC_SocialState::Solitary;
                LeavePack();
            }
            else
            {
                PackCenter = CalculatePackCenter();
            }
            break;
        }
        
        case ENPC_SocialState::Mating:
        {
            if (!bIsMatingSeason || MatingDesire < 0.3f)
            {
                CurrentSocialState = ENPC_SocialState::Solitary;
                MatingTarget = nullptr;
            }
            else
            {
                AActor* Partner = FindMatingPartner();
                if (Partner)
                {
                    MatingTarget = Partner;
                }
            }
            break;
        }
        
        case ENPC_SocialState::Territorial:
        {
            // Defend territory from intruders
            // Return to normal state after some time
            static float TerritorialTimer = 0.0f;
            TerritorialTimer += SocialUpdateInterval;
            if (TerritorialTimer > 30.0f)
            {
                CurrentSocialState = ENPC_SocialState::Solitary;
                TerritorialTimer = 0.0f;
            }
            break;
        }
        
        case ENPC_SocialState::Fleeing:
        {
            // Return to normal state when safe
            // This would typically be triggered by external threat detection
            CurrentSocialState = ENPC_SocialState::Solitary;
            break;
        }
    }
}

void UNPC_DinosaurSocialBehavior::ScanForNearbyDinosaurs()
{
    if (!GetOwner() || !GetWorld()) return;
    
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
    
    PackMembers.Empty();
    
    FVector MyLocation = GetOwner()->GetActorLocation();
    
    for (AActor* Actor : FoundActors)
    {
        if (!Actor || Actor == GetOwner()) continue;
        
        // Check if it's a dinosaur (simplified check)
        if (Actor->GetName().Contains("Dino") || Actor->GetName().Contains("Rex") || Actor->GetName().Contains("Raptor"))
        {
            float Distance = FVector::Dist(MyLocation, Actor->GetActorLocation());
            
            if (Distance <= SocialRadius)
            {
                PackMembers.Add(Actor);
                
                // Add or update social memory
                AddSocialMemory(Actor, CalculateRelationship(Actor));
            }
        }
    }
}

void UNPC_DinosaurSocialBehavior::JoinPack(AActor* NewPackLeader)
{
    if (!NewPackLeader) return;
    
    PackLeader = NewPackLeader;
    CurrentSocialState = ENPC_SocialState::InPack;
    
    // Determine role based on pack size and characteristics
    if (PackMembers.Num() <= 2)
    {
        PackRole = ENPC_PackRole::Beta;
    }
    else
    {
        PackRole = ENPC_PackRole::Hunter;
    }
    
    CommunicateWithPack("JoinedPack");
    
    UE_LOG(LogTemp, Log, TEXT("%s joined pack led by %s"), *GetOwner()->GetName(), *PackLeader->GetName());
}

void UNPC_DinosaurSocialBehavior::LeavePack()
{
    if (PackLeader)
    {
        CommunicateWithPack("LeavingPack");
    }
    
    PackLeader = nullptr;
    PackMembers.Empty();
    PackRole = ENPC_PackRole::Hunter;
    CurrentSocialState = ENPC_SocialState::Solitary;
    
    UE_LOG(LogTemp, Log, TEXT("%s left the pack"), *GetOwner()->GetName());
}

void UNPC_DinosaurSocialBehavior::EstablishPackLeadership()
{
    if (PackMembers.Num() > 0)
    {
        PackLeader = GetOwner();
        PackRole = ENPC_PackRole::Alpha;
        CurrentSocialState = ENPC_SocialState::InPack;
        
        CommunicateWithPack("EstablishedLeadership");
        
        UE_LOG(LogTemp, Log, TEXT("%s established pack leadership"), *GetOwner()->GetName());
    }
}

FVector UNPC_DinosaurSocialBehavior::CalculatePackCenter()
{
    if (PackMembers.Num() == 0) return GetOwner()->GetActorLocation();
    
    FVector Center = FVector::ZeroVector;
    int32 ValidMembers = 0;
    
    for (AActor* Member : PackMembers)
    {
        if (Member)
        {
            Center += Member->GetActorLocation();
            ValidMembers++;
        }
    }
    
    if (ValidMembers > 0)
    {
        Center /= ValidMembers;
    }
    
    return Center;
}

void UNPC_DinosaurSocialBehavior::AddSocialMemory(AActor* Actor, float RelationshipValue)
{
    if (!Actor) return;
    
    // Check if memory already exists
    for (FNPC_SocialMemory& Memory : SocialMemories)
    {
        if (Memory.Actor == Actor)
        {
            Memory.Relationship = RelationshipValue;
            Memory.LastInteractionTime = GetWorld()->GetTimeSeconds();
            return;
        }
    }
    
    // Add new memory
    FNPC_SocialMemory NewMemory;
    NewMemory.Actor = Actor;
    NewMemory.Relationship = RelationshipValue;
    NewMemory.LastInteractionTime = GetWorld()->GetTimeSeconds();
    NewMemory.bIsPackMember = PackMembers.Contains(Actor);
    
    SocialMemories.Add(NewMemory);
}

FNPC_SocialMemory* UNPC_DinosaurSocialBehavior::GetSocialMemory(AActor* Actor)
{
    for (FNPC_SocialMemory& Memory : SocialMemories)
    {
        if (Memory.Actor == Actor)
        {
            return &Memory;
        }
    }
    return nullptr;
}

void UNPC_DinosaurSocialBehavior::UpdateSocialMemories(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = SocialMemories.Num() - 1; i >= 0; i--)
    {
        FNPC_SocialMemory& Memory = SocialMemories[i];
        
        // Remove expired memories
        if (CurrentTime - Memory.LastInteractionTime > MaxMemoryDuration)
        {
            SocialMemories.RemoveAt(i);
            continue;
        }
        
        // Decay relationship over time
        Memory.Relationship -= MemoryDecayRate * DeltaTime;
        Memory.Relationship = FMath::Clamp(Memory.Relationship, -1.0f, 1.0f);
    }
}

void UNPC_DinosaurSocialBehavior::CommunicateWithPack(const FString& Message)
{
    if (GetWorld()->GetTimeSeconds() - LastCommunicationTime < CommunicationCooldown) return;
    
    LastCommunicationTime = GetWorld()->GetTimeSeconds();
    
    for (AActor* Member : PackMembers)
    {
        if (Member && Member != GetOwner())
        {
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Member->GetActorLocation());
            if (Distance <= CommunicationRange)
            {
                // In a full implementation, this would trigger behavior on the receiving dinosaur
                UE_LOG(LogTemp, Log, TEXT("%s communicated '%s' to %s"), *GetOwner()->GetName(), *Message, *Member->GetName());
            }
        }
    }
}

void UNPC_DinosaurSocialBehavior::RespondToPackCommunication(AActor* Sender, const FString& Message)
{
    if (!Sender) return;
    
    // Update relationship with sender
    FNPC_SocialMemory* Memory = GetSocialMemory(Sender);
    if (Memory)
    {
        Memory->Relationship += 0.1f;
        Memory->Relationship = FMath::Clamp(Memory->Relationship, -1.0f, 1.0f);
    }
    
    // Respond based on message type
    if (Message == "PackFormed")
    {
        if (CurrentSocialState == ENPC_SocialState::Seeking)
        {
            JoinPack(Sender);
        }
    }
    else if (Message == "DangerAlert")
    {
        CurrentSocialState = ENPC_SocialState::Fleeing;
    }
    
    UE_LOG(LogTemp, Log, TEXT("%s received communication '%s' from %s"), *GetOwner()->GetName(), *Message, *Sender->GetName());
}

bool UNPC_DinosaurSocialBehavior::IsInTerritory(const FVector& Location)
{
    FVector MyLocation = GetOwner()->GetActorLocation();
    float Distance = FVector::Dist(MyLocation, Location);
    return Distance <= TerritorialRadius;
}

void UNPC_DinosaurSocialBehavior::DefendTerritory(AActor* Intruder)
{
    if (!Intruder) return;
    
    CurrentSocialState = ENPC_SocialState::Territorial;
    
    // Alert pack members
    CommunicateWithPack("TerritoryThreat");
    
    UE_LOG(LogTemp, Log, TEXT("%s defending territory against %s"), *GetOwner()->GetName(), *Intruder->GetName());
}

void UNPC_DinosaurSocialBehavior::InitiateMatingBehavior()
{
    bIsMatingSeason = true;
    MatingDesire = 1.0f;
    CurrentSocialState = ENPC_SocialState::Mating;
    
    UE_LOG(LogTemp, Log, TEXT("%s initiated mating behavior"), *GetOwner()->GetName());
}

AActor* UNPC_DinosaurSocialBehavior::FindMatingPartner()
{
    for (AActor* Actor : PackMembers)
    {
        if (!Actor || Actor == GetOwner()) continue;
        
        // Simple compatibility check
        FNPC_SocialMemory* Memory = GetSocialMemory(Actor);
        if (Memory && Memory->Relationship > 0.5f)
        {
            return Actor;
        }
    }
    
    return nullptr;
}

void UNPC_DinosaurSocialBehavior::ProcessSocialInteractions()
{
    // Process ongoing social behaviors based on current state
    switch (CurrentSocialState)
    {
        case ENPC_SocialState::InPack:
            // Maintain pack cohesion
            if (PackLeader && PackLeader != GetOwner())
            {
                // Follow pack leader behavior would be implemented here
            }
            break;
            
        case ENPC_SocialState::Territorial:
            // Scan for territory intruders
            for (AActor* Actor : PackMembers)
            {
                if (Actor && !IsInTerritory(Actor->GetActorLocation()))
                {
                    DefendTerritory(Actor);
                    break;
                }
            }
            break;
    }
}

void UNPC_DinosaurSocialBehavior::UpdatePackBehavior()
{
    if (CurrentSocialState != ENPC_SocialState::InPack) return;
    
    // Update pack center
    PackCenter = CalculatePackCenter();
    
    // Maintain pack cohesion
    if (PackLeader == GetOwner())
    {
        // As pack leader, coordinate pack movements
        SendPackSignal("MaintainFormation");
    }
}

void UNPC_DinosaurSocialBehavior::HandleTerritorialBehavior()
{
    if (CurrentSocialState != ENPC_SocialState::Territorial) return;
    
    // Patrol territory boundaries
    // This would integrate with movement/navigation systems
}

void UNPC_DinosaurSocialBehavior::ProcessMatingBehavior()
{
    if (CurrentSocialState != ENPC_SocialState::Mating) return;
    
    if (MatingTarget)
    {
        // Move towards mating target
        // This would integrate with movement systems
        
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), MatingTarget->GetActorLocation());
        if (Distance < 500.0f)
        {
            // Mating interaction would occur here
            MatingDesire -= 0.1f;
        }
    }
}

float UNPC_DinosaurSocialBehavior::CalculateRelationship(AActor* OtherActor)
{
    if (!OtherActor) return 0.0f;
    
    // Base relationship calculation
    float Relationship = 0.0f;
    
    // Distance factor
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), OtherActor->GetActorLocation());
    float DistanceFactor = FMath::Clamp(1.0f - (Distance / SocialRadius), 0.0f, 1.0f);
    
    // Species compatibility (simplified)
    if (GetOwner()->GetName().Contains("Rex") && OtherActor->GetName().Contains("Rex"))
    {
        Relationship += 0.3f; // Same species bonus
    }
    else if (GetOwner()->GetName().Contains("Raptor") && OtherActor->GetName().Contains("Raptor"))
    {
        Relationship += 0.5f; // Raptors are highly social
    }
    
    return Relationship * DistanceFactor;
}

bool UNPC_DinosaurSocialBehavior::CanFormPack(AActor* OtherActor)
{
    if (!OtherActor) return false;
    
    float Relationship = CalculateRelationship(OtherActor);
    return Relationship > 0.2f && PackMembers.Num() < MaxPackSize;
}

void UNPC_DinosaurSocialBehavior::SendPackSignal(const FString& SignalType)
{
    CommunicateWithPack(SignalType);
}