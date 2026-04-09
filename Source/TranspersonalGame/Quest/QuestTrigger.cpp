// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "QuestTrigger.h"
#include "QuestManager.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

AQuestTrigger::AQuestTrigger()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create trigger sphere
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    RootComponent = TriggerSphere;
    TriggerSphere->SetSphereRadius(500.0f);
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    TriggerSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    TriggerSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Create visualization mesh
    VisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualizationMesh"));
    VisualizationMesh->SetupAttachment(RootComponent);
    VisualizationMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    VisualizationMesh->SetVisibility(false); // Hidden by default

    // Default values
    TriggerAction = ETriggerAction::StartQuest;
    EmotionalTone = EEmotionalTone::Curiosity;
    bRequirePlayerInteraction = false;
    bTriggerOnce = true;
    bShowInWorld = false;
    EmotionalIntensity = 1.0f;
    TriggerColor = FLinearColor::Blue;
    bCanGenerateDynamicQuest = false;
    DynamicQuestType = EQuestType::Discovery;

    // State
    bHasBeenTriggered = false;
    bIsActive = true;
    CurrentEmotionalIntensity = 0.0f;
    LastTriggerTime = 0.0f;

    // Bind overlap events
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuestTrigger::OnTriggerBeginOverlap);
    TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AQuestTrigger::OnTriggerEndOverlap);
}

void AQuestTrigger::BeginPlay()
{
    Super::BeginPlay();

    // Get quest manager reference
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        QuestManager = GameInstance->GetSubsystem<UQuestManager>();
    }

    // Update visualization based on settings
    UpdateVisualization();

    UE_LOG(LogTemp, Log, TEXT("QuestTrigger: Initialized trigger for quest %s at location %s"), 
           *QuestID, *GetActorLocation().ToString());
}

void AQuestTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update emotional intensity decay
    if (CurrentEmotionalIntensity > 0.0f)
    {
        CurrentEmotionalIntensity = FMath::Max(0.0f, CurrentEmotionalIntensity - DeltaTime * 0.5f);
    }

    // Check for dynamic quest generation
    if (bCanGenerateDynamicQuest && ActorsInRange.Num() > 0)
    {
        GenerateDynamicQuestIfNeeded();
    }
}

void AQuestTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                         UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
                                         bool bFromSweep, const FHitResult& SweepResult)
{
    if (!IsValidTriggeringActor(OtherActor) || !bIsActive)
    {
        return;
    }

    ActorsInRange.AddUnique(OtherActor);

    UE_LOG(LogTemp, Log, TEXT("QuestTrigger: Actor %s entered trigger range for quest %s"), 
           *OtherActor->GetName(), *QuestID);

    // If no interaction required, trigger immediately
    if (!bRequirePlayerInteraction)
    {
        ActivateTrigger(OtherActor);
    }
}

void AQuestTrigger::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                       UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    ActorsInRange.Remove(OtherActor);

    UE_LOG(LogTemp, Log, TEXT("QuestTrigger: Actor %s left trigger range for quest %s"), 
           *OtherActor->GetName(), *QuestID);
}

void AQuestTrigger::ActivateTrigger(AActor* TriggeringActor)
{
    if (!bIsActive || (bTriggerOnce && bHasBeenTriggered))
    {
        return;
    }

    if (!IsValidTriggeringActor(TriggeringActor))
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("QuestTrigger: Activating trigger for quest %s by actor %s"), 
           *QuestID, *TriggeringActor->GetName());

    // Process the quest action
    ProcessQuestAction(TriggeringActor);

    // Play emotional feedback
    PlayEmotionalFeedback();

    // Update emotional state
    UpdateEmotionalState(EmotionalTone, EmotionalIntensity);

    // Mark as triggered
    bHasBeenTriggered = true;
    LastTriggerTime = UGameplayStatics::GetTimeSeconds(this);

    // Broadcast events
    OnQuestTriggerActivated(QuestID);
    OnEmotionalMomentTriggered(EmotionalTone, CurrentEmotionalIntensity);

    // Deactivate if single-use
    if (bTriggerOnce)
    {
        SetTriggerActive(false);
    }
}

void AQuestTrigger::SetTriggerActive(bool bActive)
{
    bIsActive = bActive;
    TriggerSphere->SetCollisionEnabled(bActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
    
    UpdateVisualization();

    UE_LOG(LogTemp, Log, TEXT("QuestTrigger: Set trigger %s to %s"), 
           *QuestID, bActive ? TEXT("Active") : TEXT("Inactive"));
}

bool AQuestTrigger::IsPlayerInRange() const
{
    for (AActor* Actor : ActorsInRange)
    {
        if (ACharacter* Character = Cast<ACharacter>(Actor))
        {
            // Check if this is the player character
            if (Character->IsPlayerControlled())
            {
                return true;
            }
        }
    }
    return false;
}

void AQuestTrigger::UpdateEmotionalState(EEmotionalTone NewTone, float Intensity)
{
    EmotionalTone = NewTone;
    CurrentEmotionalIntensity = FMath::Clamp(Intensity, 0.0f, 2.0f);

    // Update visual representation based on emotional state
    FLinearColor EmotionalColor;
    switch (EmotionalTone)
    {
        case EEmotionalTone::Fear:
            EmotionalColor = FLinearColor::Red;
            break;
        case EEmotionalTone::Wonder:
            EmotionalColor = FLinearColor::Blue;
            break;
        case EEmotionalTone::Hope:
            EmotionalColor = FLinearColor::Green;
            break;
        case EEmotionalTone::Desperation:
            EmotionalColor = FLinearColor(0.5f, 0.0f, 0.0f); // Dark red
            break;
        case EEmotionalTone::Triumph:
            EmotionalColor = FLinearColor::Yellow;
            break;
        default:
            EmotionalColor = TriggerColor;
            break;
    }

    TriggerColor = EmotionalColor;
    UpdateVisualization();
}

void AQuestTrigger::ProcessQuestAction(AActor* TriggeringActor)
{
    if (!QuestManager)
    {
        UE_LOG(LogTemp, Error, TEXT("QuestTrigger: QuestManager not found"));
        return;
    }

    switch (TriggerAction)
    {
        case ETriggerAction::StartQuest:
        {
            bool bSuccess = QuestManager->StartQuest(QuestID);
            UE_LOG(LogTemp, Log, TEXT("QuestTrigger: %s quest %s"), 
                   bSuccess ? TEXT("Started") : TEXT("Failed to start"), *QuestID);
            break;
        }
        case ETriggerAction::UpdateObjective:
        {
            QuestManager->UpdateObjectiveProgress(QuestID, ObjectiveID, 1);
            UE_LOG(LogTemp, Log, TEXT("QuestTrigger: Updated objective %s in quest %s"), 
                   *ObjectiveID, *QuestID);
            break;
        }
        case ETriggerAction::CompleteObjective:
        {
            // Complete the objective by setting progress to target
            if (UQuestInstance* QuestInstance = QuestManager->GetQuestInstance(QuestID))
            {
                FQuestObjective Objective = QuestInstance->GetObjective(ObjectiveID);
                QuestManager->UpdateObjectiveProgress(QuestID, ObjectiveID, Objective.TargetCount);
            }
            break;
        }
        case ETriggerAction::CompleteQuest:
        {
            bool bSuccess = QuestManager->CompleteQuest(QuestID);
            UE_LOG(LogTemp, Log, TEXT("QuestTrigger: %s quest %s"), 
                   bSuccess ? TEXT("Completed") : TEXT("Failed to complete"), *QuestID);
            break;
        }
        case ETriggerAction::FailQuest:
        {
            bool bSuccess = QuestManager->FailQuest(QuestID);
            UE_LOG(LogTemp, Log, TEXT("QuestTrigger: %s quest %s"), 
                   bSuccess ? TEXT("Failed") : TEXT("Could not fail"), *QuestID);
            break;
        }
        case ETriggerAction::TriggerEmotional:
        {
            QuestManager->TriggerEmotionalMoment(QuestID, EmotionalTone, TriggerMessage.ToString());
            break;
        }
        case ETriggerAction::GenerateQuest:
        {
            GenerateDynamicQuestIfNeeded();
            break;
        }
    }
}

void AQuestTrigger::PlayEmotionalFeedback()
{
    // Play sound
    if (TriggerSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, TriggerSound, GetActorLocation());
    }

    // Spawn particle effect
    if (TriggerEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(this, TriggerEffect, GetActorLocation());
    }

    // Show message to player
    if (!TriggerMessage.IsEmpty())
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, TriggerColor.ToFColor(true), TriggerMessage.ToString());
        }
    }
}

void AQuestTrigger::GenerateDynamicQuestIfNeeded()
{
    if (!QuestManager || !bCanGenerateDynamicQuest)
    {
        return;
    }

    // Check if we should generate a new quest
    float TimeSinceLastTrigger = UGameplayStatics::GetTimeSeconds(this) - LastTriggerTime;
    if (TimeSinceLastTrigger < 60.0f) // Don't generate too frequently
    {
        return;
    }

    FString GeneratedQuestID;

    switch (DynamicQuestType)
    {
        case EQuestType::Survival:
        {
            GeneratedQuestID = QuestManager->GenerateSurvivalQuest(EmotionalTone, GetActorLocation());
            break;
        }
        case EQuestType::Discovery:
        {
            if (PossibleDinosaurSpecies.Num() > 0)
            {
                FString RandomSpecies = PossibleDinosaurSpecies[FMath::RandRange(0, PossibleDinosaurSpecies.Num() - 1)];
                GeneratedQuestID = QuestManager->GenerateDiscoveryQuest(RandomSpecies, GetActorLocation());
            }
            break;
        }
        case EQuestType::Emergency:
        {
            GeneratedQuestID = QuestManager->GenerateEmergencyQuest(TEXT("Predator"), GetActorLocation());
            break;
        }
    }

    if (!GeneratedQuestID.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("QuestTrigger: Generated dynamic quest %s"), *GeneratedQuestID);
        
        // Optionally auto-start the generated quest
        QuestManager->StartQuest(GeneratedQuestID);
    }
}

bool AQuestTrigger::IsValidTriggeringActor(AActor* Actor) const
{
    if (!Actor)
    {
        return false;
    }

    // Check if it's a player character
    ACharacter* Character = Cast<ACharacter>(Actor);
    if (Character && Character->IsPlayerControlled())
    {
        return true;
    }

    return false;
}

void AQuestTrigger::UpdateVisualization()
{
    if (!VisualizationMesh)
    {
        return;
    }

    // Show/hide visualization based on settings
    VisualizationMesh->SetVisibility(bShowInWorld && bIsActive);

    // Update color based on emotional state and activity
    if (VisualizationMesh->GetMaterial(0))
    {
        // Create dynamic material instance to change color
        UMaterialInstanceDynamic* DynamicMaterial = VisualizationMesh->CreateDynamicMaterialInstance(0);
        if (DynamicMaterial)
        {
            FLinearColor FinalColor = TriggerColor;
            FinalColor.A = bIsActive ? 0.7f : 0.3f; // Fade if inactive
            
            DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), FinalColor);
            DynamicMaterial->SetScalarParameterValue(TEXT("EmotionalIntensity"), CurrentEmotionalIntensity);
        }
    }

    // Update trigger sphere visualization in editor
    #if WITH_EDITOR
    TriggerSphere->SetHiddenInGame(!bShowInWorld);
    #endif
}