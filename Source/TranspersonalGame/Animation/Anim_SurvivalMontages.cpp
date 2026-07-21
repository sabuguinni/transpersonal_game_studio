#include "Anim_SurvivalMontages.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"

UAnim_SurvivalMontages::UAnim_SurvivalMontages()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    CurrentAction = EAnim_SurvivalAction::None;
    bIsPlayingMontage = false;
}

void UAnim_SurvivalMontages::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
    if (!OwnerMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAnim_SurvivalMontages: No SkeletalMeshComponent found on owner"));
        return;
    }

    InitializeDefaultMontages();
}

void UAnim_SurvivalMontages::InitializeDefaultMontages()
{
    // Initialize default montage data for each survival action
    // These will be set up in Blueprint or loaded from assets
    
    FAnim_MontageData GatherWoodData;
    GatherWoodData.PlayRate = 1.0f;
    GatherWoodData.BlendInTime = 0.3f;
    GatherWoodData.BlendOutTime = 0.3f;
    SurvivalMontages.Add(EAnim_SurvivalAction::GatherWood, GatherWoodData);

    FAnim_MontageData CraftToolData;
    CraftToolData.PlayRate = 0.8f;
    CraftToolData.BlendInTime = 0.25f;
    CraftToolData.BlendOutTime = 0.25f;
    SurvivalMontages.Add(EAnim_SurvivalAction::CraftTool, CraftToolData);

    FAnim_MontageData HuntSpearData;
    HuntSpearData.PlayRate = 1.2f;
    HuntSpearData.BlendInTime = 0.2f;
    HuntSpearData.BlendOutTime = 0.2f;
    SurvivalMontages.Add(EAnim_SurvivalAction::HuntSpear, HuntSpearData);

    FAnim_MontageData BuildShelterData;
    BuildShelterData.PlayRate = 0.9f;
    BuildShelterData.BlendInTime = 0.4f;
    BuildShelterData.BlendOutTime = 0.4f;
    SurvivalMontages.Add(EAnim_SurvivalAction::BuildShelter, BuildShelterData);

    FAnim_MontageData EatFoodData;
    EatFoodData.PlayRate = 1.0f;
    EatFoodData.BlendInTime = 0.2f;
    EatFoodData.BlendOutTime = 0.3f;
    SurvivalMontages.Add(EAnim_SurvivalAction::EatFood, EatFoodData);

    FAnim_MontageData DrinkWaterData;
    DrinkWaterData.PlayRate = 1.1f;
    DrinkWaterData.BlendInTime = 0.2f;
    DrinkWaterData.BlendOutTime = 0.2f;
    SurvivalMontages.Add(EAnim_SurvivalAction::DrinkWater, DrinkWaterData);

    FAnim_MontageData MakeFireData;
    MakeFireData.PlayRate = 0.7f;
    MakeFireData.BlendInTime = 0.5f;
    MakeFireData.BlendOutTime = 0.5f;
    SurvivalMontages.Add(EAnim_SurvivalAction::MakeFire, MakeFireData);

    FAnim_MontageData SkinAnimalData;
    SkinAnimalData.PlayRate = 0.8f;
    SkinAnimalData.BlendInTime = 0.3f;
    SkinAnimalData.BlendOutTime = 0.4f;
    SurvivalMontages.Add(EAnim_SurvivalAction::SkinAnimal, SkinAnimalData);
}

bool UAnim_SurvivalMontages::PlaySurvivalAction(EAnim_SurvivalAction Action)
{
    if (!OwnerMesh || !OwnerMesh->GetAnimInstance())
    {
        UE_LOG(LogTemp, Warning, TEXT("UAnim_SurvivalMontages: Cannot play action - no valid mesh or anim instance"));
        return false;
    }

    if (bIsPlayingMontage && CurrentAction != EAnim_SurvivalAction::None)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAnim_SurvivalMontages: Already playing an action"));
        return false;
    }

    FAnim_MontageData* MontageData = SurvivalMontages.Find(Action);
    if (!MontageData || !MontageData->Montage)
    {
        UE_LOG(LogTemp, Warning, TEXT("UAnim_SurvivalMontages: No montage found for action"));
        return false;
    }

    UAnimInstance* AnimInstance = OwnerMesh->GetAnimInstance();
    float MontageLength = AnimInstance->Montage_Play(
        MontageData->Montage,
        MontageData->PlayRate,
        EMontagePlayReturnType::MontageLength,
        0.0f,
        true
    );

    if (MontageLength > 0.0f)
    {
        CurrentAction = Action;
        bIsPlayingMontage = true;

        // Bind to montage end event
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &UAnim_SurvivalMontages::OnMontageEnded);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, MontageData->Montage);

        OnSurvivalActionStarted(Action);
        
        UE_LOG(LogTemp, Log, TEXT("UAnim_SurvivalMontages: Started playing action %d"), (int32)Action);
        return true;
    }

    return false;
}

void UAnim_SurvivalMontages::StopCurrentAction()
{
    if (!OwnerMesh || !OwnerMesh->GetAnimInstance() || !bIsPlayingMontage)
        return;

    UAnimInstance* AnimInstance = OwnerMesh->GetAnimInstance();
    AnimInstance->Montage_Stop(0.25f);
    
    CurrentAction = EAnim_SurvivalAction::None;
    bIsPlayingMontage = false;
}

bool UAnim_SurvivalMontages::IsActionAvailable(EAnim_SurvivalAction Action) const
{
    if (bIsPlayingMontage)
        return false;

    FAnim_MontageData const* MontageData = SurvivalMontages.Find(Action);
    return MontageData && MontageData->Montage;
}

float UAnim_SurvivalMontages::GetActionDuration(EAnim_SurvivalAction Action) const
{
    FAnim_MontageData const* MontageData = SurvivalMontages.Find(Action);
    if (MontageData && MontageData->Montage)
    {
        return MontageData->Montage->GetPlayLength() / MontageData->PlayRate;
    }
    return 0.0f;
}

void UAnim_SurvivalMontages::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    EAnim_SurvivalAction CompletedAction = CurrentAction;
    
    CurrentAction = EAnim_SurvivalAction::None;
    bIsPlayingMontage = false;

    if (!bInterrupted)
    {
        OnSurvivalActionCompleted(CompletedAction);
    }

    UE_LOG(LogTemp, Log, TEXT("UAnim_SurvivalMontages: Action completed %d (interrupted: %s)"), 
           (int32)CompletedAction, bInterrupted ? TEXT("true") : TEXT("false"));
}