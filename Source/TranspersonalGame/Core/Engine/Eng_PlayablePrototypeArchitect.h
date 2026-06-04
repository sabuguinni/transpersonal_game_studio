#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "SharedTypes.h"
#include "Eng_PlayablePrototypeArchitect.generated.h"

UENUM(BlueprintType)
enum class EEng_PrototypeComponent : uint8
{
    None = 0,
    ThirdPersonCharacter = 1,
    CameraBoom = 2,
    FollowCamera = 3,
    Landscape = 4,
    PlayerMovement = 5,
    DinosaurMeshes = 6,
    DirectionalLight = 7,
    SkyAtmosphere = 8,
    ExponentialHeightFog = 9,
    PlayerStart = 10,
    NavMeshBounds = 11
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PrototypeRequirement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype")
    EEng_PrototypeComponent Component;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype")
    bool bIsRequired;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype")
    bool bIsImplemented;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype")
    float Priority;

    FEng_PrototypeRequirement()
    {
        Component = EEng_PrototypeComponent::None;
        bIsRequired = false;
        bIsImplemented = false;
        Description = TEXT("");
        Priority = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_WalkAroundMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    TArray<FEng_PrototypeRequirement> Requirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    int32 CompletedComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    int32 TotalComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bMilestoneComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString StatusMessage;

    FEng_WalkAroundMilestone()
    {
        CompletionPercentage = 0.0f;
        CompletedComponents = 0;
        TotalComponents = 0;
        bMilestoneComplete = false;
        StatusMessage = TEXT("Milestone not started");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_PlayablePrototypeArchitect : public UObject
{
    GENERATED_BODY()

public:
    UEng_PlayablePrototypeArchitect();

    // Core validation methods
    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    bool ValidatePlayablePrototype();

    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    FEng_WalkAroundMilestone GetWalkAroundMilestoneStatus();

    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    bool ValidateCharacterController();

    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    bool ValidateWorldEnvironment();

    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    bool ValidateDinosaurPresence();

    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    bool ValidateLightingSetup();

    // Component-specific validation
    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    bool CheckThirdPersonCharacter();

    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    bool CheckCameraSetup();

    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    bool CheckLandscapeTerrain();

    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    bool CheckPlayerMovement();

    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    bool CheckDinosaurMeshes();

    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    bool CheckLightingComponents();

    // Milestone management
    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    void UpdateMilestoneProgress();

    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    void GeneratePrototypeReport();

    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    TArray<FString> GetMissingComponents();

    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    float CalculateCompletionPercentage();

    // Architecture enforcement
    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    bool EnforceMinimumViablePrototype();

    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture")
    bool ValidateGameplayReadiness();

    UFUNCTION(BlueprintCallable, Category = "Prototype Architecture", CallInEditor = true)
    void RunFullPrototypeValidation();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prototype State")
    FEng_WalkAroundMilestone CurrentMilestone;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prototype State")
    TArray<FEng_PrototypeRequirement> PrototypeRequirements;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prototype State")
    bool bPrototypeValidated;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prototype State")
    FString LastValidationResult;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prototype State")
    float LastValidationTime;

private:
    void InitializePrototypeRequirements();
    bool ValidateWorldActors();
    bool ValidatePlayerController();
    bool ValidateGameMode();
    void LogValidationResult(const FString& Component, bool bResult);
};