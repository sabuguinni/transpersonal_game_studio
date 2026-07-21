#include "Quest_InteractiveQuestMarker.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"

AQuest_InteractiveQuestMarker::AQuest_InteractiveQuestMarker()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Create mesh component for visual representation
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    
    // Create interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(200.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    
    // Bind overlap events
    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuest_InteractiveQuestMarker::OnPlayerEnterRange);
    InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AQuest_InteractiveQuestMarker::OnPlayerExitRange);
    
    // Initialize quest data
    QuestTitle = TEXT("Survival Mission");
    QuestDescription = TEXT("Complete this survival task to progress");
    QuestType = EQuest_MissionType::Survival;
    QuestDifficulty = EQuest_Difficulty::Normal;
    bIsActive = true;
    bIsCompleted = false;
    bPlayerInRange = false;
    ExperienceReward = 100;
    
    // Set default mesh scale
    MeshComponent->SetWorldScale3D(FVector(2.0f, 2.0f, 3.0f));
}

void AQuest_InteractiveQuestMarker::BeginPlay()
{
    Super::BeginPlay();
    
    // Set up visual appearance
    if (MeshComponent)
    {
        // Create dynamic material for glowing effect
        if (UMaterialInterface* BaseMaterial = MeshComponent->GetMaterial(0))
        {
            DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            if (DynamicMaterial)
            {
                MeshComponent->SetMaterial(0, DynamicMaterial);
                UpdateMarkerAppearance();
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Quest Marker '%s' initialized at location %s"), 
           *QuestTitle, *GetActorLocation().ToString());
}

void AQuest_InteractiveQuestMarker::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsActive && !bIsCompleted)
    {
        // Animate the marker (floating and rotation)
        AnimateMarker(DeltaTime);
        
        // Check for player interaction input
        if (bPlayerInRange)
        {
            CheckForInteraction();
        }
    }
}

void AQuest_InteractiveQuestMarker::OnPlayerEnterRange(UPrimitiveComponent* OverlappedComponent, 
                                                      AActor* OtherActor, 
                                                      UPrimitiveComponent* OtherComponent, 
                                                      int32 OtherBodyIndex, 
                                                      bool bFromSweep, 
                                                      const FHitResult& SweepResult)
{
    if (ACharacter* Player = Cast<ACharacter>(OtherActor))
    {
        bPlayerInRange = true;
        OnPlayerEnterQuestRange();
        
        // Show quest UI prompt
        if (GEngine)
        {
            FString PromptText = FString::Printf(TEXT("Press E to interact with: %s"), *QuestTitle);
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, PromptText);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Player entered range of quest marker: %s"), *QuestTitle);
    }
}

void AQuest_InteractiveQuestMarker::OnPlayerExitRange(UPrimitiveComponent* OverlappedComponent, 
                                                     AActor* OtherActor, 
                                                     UPrimitiveComponent* OtherComponent, 
                                                     int32 OtherBodyIndex)
{
    if (ACharacter* Player = Cast<ACharacter>(OtherActor))
    {
        bPlayerInRange = false;
        OnPlayerExitQuestRange();
        
        UE_LOG(LogTemp, Log, TEXT("Player exited range of quest marker: %s"), *QuestTitle);
    }
}

void AQuest_InteractiveQuestMarker::InteractWithQuest()
{
    if (!bIsActive || bIsCompleted)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Player interacted with quest: %s"), *QuestTitle);
    
    // Show quest details
    if (GEngine)
    {
        FString QuestInfo = FString::Printf(
            TEXT("Quest: %s\nDescription: %s\nDifficulty: %s\nReward: %d XP"),
            *QuestTitle,
            *QuestDescription,
            *GetDifficultyString(),
            ExperienceReward
        );
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Cyan, QuestInfo);
    }
    
    // Activate quest
    ActivateQuest();
}

void AQuest_InteractiveQuestMarker::ActivateQuest()
{
    if (bIsCompleted)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Activating quest: %s"), *QuestTitle);
    
    // Broadcast quest activation
    OnQuestActivated.Broadcast(this);
    
    // Update visual state
    UpdateMarkerAppearance();
    
    // Show activation message
    if (GEngine)
    {
        FString ActivationText = FString::Printf(TEXT("Quest Activated: %s"), *QuestTitle);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, ActivationText);
    }
}

void AQuest_InteractiveQuestMarker::CompleteQuest()
{
    if (bIsCompleted)
    {
        return;
    }
    
    bIsCompleted = true;
    UE_LOG(LogTemp, Log, TEXT("Quest completed: %s"), *QuestTitle);
    
    // Broadcast quest completion
    OnQuestCompleted.Broadcast(this, ExperienceReward);
    
    // Update visual state
    UpdateMarkerAppearance();
    
    // Show completion message
    if (GEngine)
    {
        FString CompletionText = FString::Printf(
            TEXT("Quest Completed: %s\nReward: %d XP"), 
            *QuestTitle, 
            ExperienceReward
        );
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Gold, CompletionText);
    }
    
    // Disable tick after completion
    SetActorTickEnabled(false);
}

void AQuest_InteractiveQuestMarker::UpdateMarkerAppearance()
{
    if (!MeshComponent)
    {
        return;
    }
    
    if (bIsCompleted)
    {
        // Completed quest - gold color
        if (DynamicMaterial)
        {
            DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::Yellow);
            DynamicMaterial->SetScalarParameterValue(TEXT("EmissiveStrength"), 2.0f);
        }
    }
    else if (bIsActive)
    {
        // Active quest - blue color
        if (DynamicMaterial)
        {
            DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::Blue);
            DynamicMaterial->SetScalarParameterValue(TEXT("EmissiveStrength"), 1.5f);
        }
    }
    else
    {
        // Inactive quest - gray color
        if (DynamicMaterial)
        {
            DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::Gray);
            DynamicMaterial->SetScalarParameterValue(TEXT("EmissiveStrength"), 0.5f);
        }
    }
}

void AQuest_InteractiveQuestMarker::AnimateMarker(float DeltaTime)
{
    if (!MeshComponent)
    {
        return;
    }
    
    // Floating animation
    static float FloatTime = 0.0f;
    FloatTime += DeltaTime;
    
    FVector CurrentLocation = MeshComponent->GetRelativeLocation();
    float FloatOffset = FMath::Sin(FloatTime * 2.0f) * 20.0f;
    MeshComponent->SetRelativeLocation(FVector(CurrentLocation.X, CurrentLocation.Y, FloatOffset));
    
    // Rotation animation
    FRotator CurrentRotation = MeshComponent->GetRelativeRotation();
    FRotator NewRotation = CurrentRotation + FRotator(0, DeltaTime * 45.0f, 0);
    MeshComponent->SetRelativeRotation(NewRotation);
}

void AQuest_InteractiveQuestMarker::CheckForInteraction()
{
    // Check for E key press (simplified - in real game would use input binding)
    // This is a placeholder for proper input handling
    static bool bWasEPressed = false;
    
    // In a real implementation, this would be handled through the input system
    // For now, we'll trigger interaction after a delay when player is in range
    static float InteractionTimer = 0.0f;
    InteractionTimer += GetWorld()->GetDeltaSeconds();
    
    if (InteractionTimer > 2.0f) // Auto-interact after 2 seconds for demo
    {
        InteractWithQuest();
        InteractionTimer = 0.0f;
    }
}

FString AQuest_InteractiveQuestMarker::GetDifficultyString() const
{
    switch (QuestDifficulty)
    {
        case EQuest_Difficulty::Easy: return TEXT("Easy");
        case EQuest_Difficulty::Normal: return TEXT("Normal");
        case EQuest_Difficulty::Hard: return TEXT("Hard");
        case EQuest_Difficulty::Expert: return TEXT("Expert");
        default: return TEXT("Unknown");
    }
}

void AQuest_InteractiveQuestMarker::OnPlayerEnterQuestRange_Implementation()
{
    // Blueprint implementable event
}

void AQuest_InteractiveQuestMarker::OnPlayerExitQuestRange_Implementation()
{
    // Blueprint implementable event
}