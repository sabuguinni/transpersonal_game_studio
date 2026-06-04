#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerStart.h"
#include "Landscape/Landscape.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "SharedTypes.h"
#include "Eng_PlayablePrototypeValidator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PrototypeRequirement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype")
    FString RequirementName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype")
    FString ValidationDetails;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype")
    int32 CurrentCount;

    FEng_PrototypeRequirement()
    {
        RequirementName = TEXT("");
        bIsCompleted = false;
        CompletionPercentage = 0.0f;
        ValidationDetails = TEXT("");
        RequiredCount = 1;
        CurrentCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_MilestoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FEng_PrototypeRequirement> Requirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float OverallCompletion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsMilestoneComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FDateTime LastValidationTime;

    FEng_MilestoneData()
    {
        MilestoneName = TEXT("Walk Around Prototype");
        OverallCompletion = 0.0f;
        bIsMilestoneComplete = false;
        LastValidationTime = FDateTime::Now();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_PlayablePrototypeValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_PlayablePrototypeValidator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Prototype Validation")
    void ValidateWalkAroundMilestone();

    UFUNCTION(BlueprintCallable, Category = "Prototype Validation")
    FEng_MilestoneData GetCurrentMilestoneStatus();

    UFUNCTION(BlueprintCallable, Category = "Prototype Validation")
    float GetOverallPrototypeCompletion();

    UFUNCTION(BlueprintCallable, Category = "Prototype Validation")
    bool IsPrototypePlayable();

    UFUNCTION(BlueprintCallable, Category = "Prototype Validation")
    void GenerateValidationReport();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Prototype Validation")
    void RunFullValidation();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Milestone Data")
    FEng_MilestoneData CurrentMilestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    bool bAutoValidateOnPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    bool bLogValidationDetails;

private:
    void ValidateCharacterMovement();
    void ValidateTerrain();
    void ValidateDinosaurActors();
    void ValidateLighting();
    void ValidatePlayerStart();
    void ValidateCamera();
    void ValidateHUD();

    bool CheckActorExists(UClass* ActorClass);
    int32 CountActorsOfClass(UClass* ActorClass);
    bool ValidateActorComponent(AActor* Actor, UClass* ComponentClass);

    FTimerHandle ValidationTimer;
};