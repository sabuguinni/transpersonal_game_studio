#include "Quest_SurvivalCraftingTutorial.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AQuest_SurvivalCraftingTutorial::AQuest_SurvivalCraftingTutorial()
{
    PrimaryActorTick.bCanEverTick = false;

    // Criar componente raiz
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Configurar trigger sphere
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetupAttachment(RootComponent);
    TriggerSphere->SetSphereRadius(500.0f);
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Configurar mesh do marker
    QuestMarkerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("QuestMarkerMesh"));
    QuestMarkerMesh->SetupAttachment(RootComponent);
    QuestMarkerMesh->SetRelativeScale3D(FVector(2.0f, 2.0f, 4.0f));

    // Configurar texto do quest
    QuestText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("QuestText"));
    QuestText->SetupAttachment(RootComponent);
    QuestText->SetRelativeLocation(FVector(0.0f, 0.0f, 250.0f));
    QuestText->SetText(FText::FromString(TEXT("Survival Crafting Tutorial")));
    QuestText->SetTextRenderColor(FColor::Yellow);
    QuestText->SetWorldSize(100.0f);
    QuestText->SetHorizontalAlignment(EHTA_Center);

    // Configurações padrão
    CurrentStage = EQuest_CraftingTutorialStage::NotStarted;
    TriggerRadius = 500.0f;
    bIsActive = true;
    ActiveColor = FLinearColor::Yellow;
    CompletedColor = FLinearColor::Green;
    bPlayerInRange = false;
    CurrentPlayer = nullptr;

    // Bind eventos do trigger
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AQuest_SurvivalCraftingTutorial::OnTriggerBeginOverlap);
    TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AQuest_SurvivalCraftingTutorial::OnTriggerEndOverlap);
}

void AQuest_SurvivalCraftingTutorial::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeCraftingSteps();
    UpdateQuestDisplay();
    SetMarkerColor(ActiveColor);
}

void AQuest_SurvivalCraftingTutorial::InitializeCraftingSteps()
{
    CraftingSteps.Empty();

    // Passo 1: Coletar gravetos
    FQuest_CraftingStep StickStep;
    StickStep.StepName = TEXT("Collect Sticks");
    StickStep.Description = TEXT("Find and collect 3 wooden sticks from the ground");
    StickStep.RequiredAmount = 3;
    StickStep.CurrentAmount = 0;
    StickStep.bIsCompleted = false;
    CraftingSteps.Add(StickStep);

    // Passo 2: Coletar pedras
    FQuest_CraftingStep StoneStep;
    StoneStep.StepName = TEXT("Collect Stones");
    StoneStep.Description = TEXT("Find and collect 2 sharp stones");
    StoneStep.RequiredAmount = 2;
    StoneStep.CurrentAmount = 0;
    StoneStep.bIsCompleted = false;
    CraftingSteps.Add(StoneStep);

    // Passo 3: Criar machado de pedra
    FQuest_CraftingStep AxeStep;
    AxeStep.StepName = TEXT("Craft Stone Axe");
    AxeStep.Description = TEXT("Combine sticks and stones to create a stone axe");
    AxeStep.RequiredAmount = 1;
    AxeStep.CurrentAmount = 0;
    AxeStep.bIsCompleted = false;
    CraftingSteps.Add(AxeStep);

    // Passo 4: Usar machado
    FQuest_CraftingStep UseAxeStep;
    UseAxeStep.StepName = TEXT("Use Axe");
    UseAxeStep.Description = TEXT("Use the stone axe to cut down a tree");
    UseAxeStep.RequiredAmount = 1;
    UseAxeStep.CurrentAmount = 0;
    UseAxeStep.bIsCompleted = false;
    CraftingSteps.Add(UseAxeStep);

    // Passo 5: Construir abrigo
    FQuest_CraftingStep ShelterStep;
    ShelterStep.StepName = TEXT("Build Shelter");
    ShelterStep.Description = TEXT("Use collected materials to build a basic shelter");
    ShelterStep.RequiredAmount = 1;
    ShelterStep.CurrentAmount = 0;
    ShelterStep.bIsCompleted = false;
    CraftingSteps.Add(ShelterStep);
}

void AQuest_SurvivalCraftingTutorial::StartTutorial()
{
    if (CurrentStage == EQuest_CraftingTutorialStage::NotStarted)
    {
        CurrentStage = EQuest_CraftingTutorialStage::CollectSticks;
        bIsActive = true;
        UpdateQuestDisplay();
        OnTutorialStarted();
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
                TEXT("Survival Crafting Tutorial Started! Collect 3 sticks to begin."));
        }
    }
}

void AQuest_SurvivalCraftingTutorial::AdvanceStage()
{
    switch (CurrentStage)
    {
        case EQuest_CraftingTutorialStage::CollectSticks:
            if (IsStepCompleted(TEXT("Collect Sticks")))
            {
                CurrentStage = EQuest_CraftingTutorialStage::CollectStones;
            }
            break;
            
        case EQuest_CraftingTutorialStage::CollectStones:
            if (IsStepCompleted(TEXT("Collect Stones")))
            {
                CurrentStage = EQuest_CraftingTutorialStage::CraftAxe;
            }
            break;
            
        case EQuest_CraftingTutorialStage::CraftAxe:
            if (IsStepCompleted(TEXT("Craft Stone Axe")))
            {
                CurrentStage = EQuest_CraftingTutorialStage::UseAxe;
            }
            break;
            
        case EQuest_CraftingTutorialStage::UseAxe:
            if (IsStepCompleted(TEXT("Use Axe")))
            {
                CurrentStage = EQuest_CraftingTutorialStage::BuildShelter;
            }
            break;
            
        case EQuest_CraftingTutorialStage::BuildShelter:
            if (IsStepCompleted(TEXT("Build Shelter")))
            {
                CurrentStage = EQuest_CraftingTutorialStage::Completed;
                SetMarkerColor(CompletedColor);
                OnTutorialCompleted();
                
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, 
                        TEXT("Congratulations! You've completed the Survival Crafting Tutorial!"));
                }
            }
            break;
            
        default:
            break;
    }
    
    UpdateQuestDisplay();
    OnStageAdvanced(CurrentStage);
}

void AQuest_SurvivalCraftingTutorial::UpdateStepProgress(const FString& StepName, int32 Amount)
{
    for (FQuest_CraftingStep& Step : CraftingSteps)
    {
        if (Step.StepName == StepName)
        {
            Step.CurrentAmount = FMath::Min(Step.CurrentAmount + Amount, Step.RequiredAmount);
            
            if (Step.CurrentAmount >= Step.RequiredAmount && !Step.bIsCompleted)
            {
                Step.bIsCompleted = true;
                
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                        FString::Printf(TEXT("Step Completed: %s"), *StepName));
                }
                
                // Auto-advance se o passo atual foi completado
                AdvanceStage();
            }
            
            UpdateQuestDisplay();
            break;
        }
    }
}

bool AQuest_SurvivalCraftingTutorial::IsStepCompleted(const FString& StepName) const
{
    for (const FQuest_CraftingStep& Step : CraftingSteps)
    {
        if (Step.StepName == StepName)
        {
            return Step.bIsCompleted;
        }
    }
    return false;
}

FString AQuest_SurvivalCraftingTutorial::GetCurrentObjective() const
{
    switch (CurrentStage)
    {
        case EQuest_CraftingTutorialStage::NotStarted:
            return TEXT("Approach the quest marker to start the tutorial");
            
        case EQuest_CraftingTutorialStage::CollectSticks:
            {
                for (const FQuest_CraftingStep& Step : CraftingSteps)
                {
                    if (Step.StepName == TEXT("Collect Sticks"))
                    {
                        return FString::Printf(TEXT("Collect Sticks: %d/%d"), 
                            Step.CurrentAmount, Step.RequiredAmount);
                    }
                }
            }
            break;
            
        case EQuest_CraftingTutorialStage::CollectStones:
            {
                for (const FQuest_CraftingStep& Step : CraftingSteps)
                {
                    if (Step.StepName == TEXT("Collect Stones"))
                    {
                        return FString::Printf(TEXT("Collect Stones: %d/%d"), 
                            Step.CurrentAmount, Step.RequiredAmount);
                    }
                }
            }
            break;
            
        case EQuest_CraftingTutorialStage::CraftAxe:
            return TEXT("Open crafting menu and create a Stone Axe");
            
        case EQuest_CraftingTutorialStage::UseAxe:
            return TEXT("Use your Stone Axe to cut down a tree");
            
        case EQuest_CraftingTutorialStage::BuildShelter:
            return TEXT("Build a basic shelter using collected materials");
            
        case EQuest_CraftingTutorialStage::Completed:
            return TEXT("Tutorial Completed! Well done, survivor!");
            
        default:
            return TEXT("Unknown objective");
    }
    
    return TEXT("Unknown objective");
}

float AQuest_SurvivalCraftingTutorial::GetOverallProgress() const
{
    if (CurrentStage == EQuest_CraftingTutorialStage::Completed)
    {
        return 1.0f;
    }
    
    int32 CompletedSteps = 0;
    for (const FQuest_CraftingStep& Step : CraftingSteps)
    {
        if (Step.bIsCompleted)
        {
            CompletedSteps++;
        }
    }
    
    return CraftingSteps.Num() > 0 ? (float)CompletedSteps / (float)CraftingSteps.Num() : 0.0f;
}

void AQuest_SurvivalCraftingTutorial::UpdateQuestDisplay()
{
    if (QuestText)
    {
        FString DisplayText = GetCurrentObjective();
        float Progress = GetOverallProgress() * 100.0f;
        
        FString FullText = FString::Printf(TEXT("Survival Crafting Tutorial\n%s\nProgress: %.0f%%"), 
            *DisplayText, Progress);
        
        QuestText->SetText(FText::FromString(FullText));
    }
}

void AQuest_SurvivalCraftingTutorial::SetMarkerColor(const FLinearColor& Color)
{
    if (QuestMarkerMesh && QuestMarkerMesh->GetMaterial(0))
    {
        // Criar dynamic material instance para mudar cor
        UMaterialInstanceDynamic* DynamicMaterial = 
            QuestMarkerMesh->CreateAndSetMaterialInstanceDynamic(0);
        
        if (DynamicMaterial)
        {
            DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
        }
    }
}

void AQuest_SurvivalCraftingTutorial::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, 
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (APawn* PlayerPawn = Cast<APawn>(OtherActor))
    {
        if (PlayerPawn->IsPlayerControlled())
        {
            bPlayerInRange = true;
            CurrentPlayer = PlayerPawn;
            
            OnPlayerEnteredTrigger(PlayerPawn);
            
            // Auto-start tutorial se ainda não começou
            if (CurrentStage == EQuest_CraftingTutorialStage::NotStarted)
            {
                StartTutorial();
            }
            
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, 
                    TEXT("Entered Crafting Tutorial Area"));
            }
        }
    }
}

void AQuest_SurvivalCraftingTutorial::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (APawn* PlayerPawn = Cast<APawn>(OtherActor))
    {
        if (PlayerPawn->IsPlayerControlled() && PlayerPawn == CurrentPlayer)
        {
            bPlayerInRange = false;
            CurrentPlayer = nullptr;
            
            OnPlayerExitedTrigger(PlayerPawn);
            
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, 
                    TEXT("Left Crafting Tutorial Area"));
            }
        }
    }
}