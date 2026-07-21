#include "Narr_NPCCharacter.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ANarr_NPCCharacter::ANarr_NPCCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(200.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Bind overlap events
    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_NPCCharacter::OnInteractionSphereBeginOverlap);
    InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ANarr_NPCCharacter::OnInteractionSphereEndOverlap);

    // Initialize NPC data
    NPCData.NPCName = TEXT("Survivor");
    NPCData.NPCType = ENarr_NPCType::Survivor;
    NPCData.InteractionRange = 200.0f;
    NPCData.bCanTrade = false;
    NPCData.bCanGiveQuests = true;

    // Initialize state
    CurrentState = ENarr_NPCState::Idle;
    CurrentInteractingPlayer = nullptr;
    PatrolRadius = 1000.0f;
    MovementSpeed = 150.0f;
    StateTimer = 0.0f;

    // Configure movement
    GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
}

void ANarr_NPCCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    HomeLocation = GetActorLocation();
    CurrentPatrolTarget = HomeLocation;
    
    // Add initial dialogue options based on NPC type
    switch (NPCData.NPCType)
    {
        case ENarr_NPCType::Survivor:
            NPCData.DialogueOptions.Add(TEXT("I've been hiding from the predators. This place isn't safe."));
            NPCData.DialogueOptions.Add(TEXT("Water source to the east, but watch for raptors."));
            break;
        case ENarr_NPCType::Hunter:
            NPCData.DialogueOptions.Add(TEXT("Track the herbivores at dawn. They drink at the river."));
            NPCData.DialogueOptions.Add(TEXT("Sharp stones make the best spear points."));
            break;
        case ENarr_NPCType::Crafter:
            NPCData.DialogueOptions.Add(TEXT("Gather dry wood before the rains come."));
            NPCData.DialogueOptions.Add(TEXT("Fire keeps the night hunters away."));
            break;
        case ENarr_NPCType::Guide:
            NPCData.DialogueOptions.Add(TEXT("The mountain pass is treacherous but leads to safety."));
            NPCData.DialogueOptions.Add(TEXT("Follow the star patterns to navigate at night."));
            break;
        case ENarr_NPCType::Trader:
            NPCData.DialogueOptions.Add(TEXT("Fresh meat for worked stone - fair trade."));
            NPCData.DialogueOptions.Add(TEXT("I have supplies, but nothing comes free."));
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("NPCCharacter: %s initialized with %d dialogue options"), 
           *NPCData.NPCName, NPCData.DialogueOptions.Num());
}

void ANarr_NPCCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    StateTimer += DeltaTime;
    
    switch (CurrentState)
    {
        case ENarr_NPCState::Patrolling:
            UpdatePatrolling(DeltaTime);
            break;
        case ENarr_NPCState::Fleeing:
            UpdateFleeing(DeltaTime);
            break;
        case ENarr_NPCState::Idle:
            // Switch to patrolling after idle period
            if (StateTimer > 5.0f)
            {
                SetNPCState(ENarr_NPCState::Patrolling);
            }
            break;
    }
}

void ANarr_NPCCharacter::StartDialogue(AActor* Player)
{
    if (!CanInteract() || !Player)
        return;

    CurrentInteractingPlayer = Player;
    SetNPCState(ENarr_NPCState::Talking);
    
    // Stop movement
    GetCharacterMovement()->StopMovementImmediately();
    
    // Face the player
    FVector DirectionToPlayer = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FRotator LookRotation = FRotationMatrix::MakeFromX(DirectionToPlayer).Rotator();
    LookRotation.Pitch = 0.0f;
    LookRotation.Roll = 0.0f;
    SetActorRotation(LookRotation);
    
    // Play random dialogue
    if (NPCData.DialogueOptions.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, NPCData.DialogueOptions.Num() - 1);
        FString DialogueText = NPCData.DialogueOptions[RandomIndex];
        
        UE_LOG(LogTemp, Warning, TEXT("NPCCharacter: %s says: %s"), 
               *NPCData.NPCName, *DialogueText);
    }
}

void ANarr_NPCCharacter::EndDialogue()
{
    CurrentInteractingPlayer = nullptr;
    SetNPCState(ENarr_NPCState::Idle);
    
    UE_LOG(LogTemp, Warning, TEXT("NPCCharacter: %s ended dialogue"), *NPCData.NPCName);
}

void ANarr_NPCCharacter::SetNPCState(ENarr_NPCState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateTimer = 0.0f;
        
        switch (NewState)
        {
            case ENarr_NPCState::Patrolling:
                GenerateNewPatrolTarget();
                break;
            case ENarr_NPCState::Talking:
                GetCharacterMovement()->StopMovementImmediately();
                break;
        }
    }
}

bool ANarr_NPCCharacter::CanInteract() const
{
    return CurrentState != ENarr_NPCState::Fleeing && CurrentState != ENarr_NPCState::Talking;
}

void ANarr_NPCCharacter::FleeFromDanger(FVector DangerLocation)
{
    SetNPCState(ENarr_NPCState::Fleeing);
    
    // Calculate flee direction (away from danger)
    FVector FleeDirection = (GetActorLocation() - DangerLocation).GetSafeNormal();
    CurrentPatrolTarget = GetActorLocation() + (FleeDirection * PatrolRadius * 2.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("NPCCharacter: %s fleeing from danger"), *NPCData.NPCName);
}

void ANarr_NPCCharacter::UpdatePatrolling(float DeltaTime)
{
    float DistanceToTarget = FVector::Dist(GetActorLocation(), CurrentPatrolTarget);
    
    if (DistanceToTarget < 100.0f || StateTimer > 10.0f)
    {
        GenerateNewPatrolTarget();
    }
    else
    {
        // Move towards patrol target
        FVector Direction = (CurrentPatrolTarget - GetActorLocation()).GetSafeNormal();
        AddMovementInput(Direction, 1.0f);
    }
}

void ANarr_NPCCharacter::UpdateFleeing(float DeltaTime)
{
    float DistanceToTarget = FVector::Dist(GetActorLocation(), CurrentPatrolTarget);
    
    if (DistanceToTarget < 200.0f)
    {
        // Continue fleeing
        FVector Direction = (CurrentPatrolTarget - GetActorLocation()).GetSafeNormal();
        AddMovementInput(Direction, 1.0f);
    }
    else if (StateTimer > 15.0f)
    {
        // Stop fleeing after timeout
        SetNPCState(ENarr_NPCState::Idle);
    }
}

void ANarr_NPCCharacter::GenerateNewPatrolTarget()
{
    FVector RandomOffset = FVector(
        FMath::RandRange(-PatrolRadius, PatrolRadius),
        FMath::RandRange(-PatrolRadius, PatrolRadius),
        0.0f
    );
    
    CurrentPatrolTarget = HomeLocation + RandomOffset;
    StateTimer = 0.0f;
}

void ANarr_NPCCharacter::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, 
                                                        AActor* OtherActor, UPrimitiveComponent* OtherComp, 
                                                        int32 OtherBodyIndex, bool bFromSweep, 
                                                        const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA<APawn>() && CanInteract())
    {
        UE_LOG(LogTemp, Warning, TEXT("NPCCharacter: Player entered interaction range of %s"), *NPCData.NPCName);
    }
}

void ANarr_NPCCharacter::OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, 
                                                      AActor* OtherActor, UPrimitiveComponent* OtherComp, 
                                                      int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor == CurrentInteractingPlayer)
    {
        EndDialogue();
    }
}