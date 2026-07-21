#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/NoExportTypes.h"
#include "Eng_PlayablePrototype.generated.h"

UENUM(BlueprintType)
enum class EEng_PrototypeState : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct FEng_PrototypeRequirement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
    FString RequirementName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
    float Priority;

    FEng_PrototypeRequirement()
    {
        RequirementName = TEXT("");
        bIsCompleted = false;
        Description = TEXT("");
        Priority = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FEng_WalkAroundMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bHasThirdPersonCharacter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bHasWASDMovement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bHasCameraBoom;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bHasLandscape;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bHasDinosaurMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bHasLighting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    float TerrainVariation;

    FEng_WalkAroundMilestone()
    {
        bHasThirdPersonCharacter = false;
        bHasWASDMovement = false;
        bHasCameraBoom = false;
        bHasLandscape = false;
        bHasDinosaurMeshes = false;
        bHasLighting = false;
        DinosaurCount = 0;
        TerrainVariation = 0.0f;
    }
};

/**
 * Engine Architect's Playable Prototype Manager
 * Validates and tracks WALK AROUND milestone requirements
 * Ensures minimum viable playable prototype is achieved
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_PlayablePrototype : public UObject
{
    GENERATED_BODY()

public:
    UEng_PlayablePrototype();

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "Prototype Validation")
    bool ValidateWalkAroundMilestone();

    UFUNCTION(BlueprintCallable, Category = "Prototype Validation")
    FEng_WalkAroundMilestone GetCurrentMilestoneStatus();

    UFUNCTION(BlueprintCallable, Category = "Prototype Validation")
    TArray<FEng_PrototypeRequirement> GetMissingRequirements();

    UFUNCTION(BlueprintCallable, Category = "Prototype Validation")
    float GetMilestoneCompletionPercentage();

    // Character validation
    UFUNCTION(BlueprintCallable, Category = "Character Validation")
    bool ValidateThirdPersonCharacter();

    UFUNCTION(BlueprintCallable, Category = "Character Validation")
    bool ValidateCharacterMovement();

    UFUNCTION(BlueprintCallable, Category = "Character Validation")
    bool ValidateCameraSystem();

    // World validation
    UFUNCTION(BlueprintCallable, Category = "World Validation")
    bool ValidateLandscapeTerrain();

    UFUNCTION(BlueprintCallable, Category = "World Validation")
    bool ValidateDinosaurMeshes();

    UFUNCTION(BlueprintCallable, Category = "World Validation")
    bool ValidateLightingSystem();

    // Diagnostic functions
    UFUNCTION(BlueprintCallable, Category = "Diagnostics")
    void GeneratePrototypeReport();

    UFUNCTION(BlueprintCallable, Category = "Diagnostics")
    void LogMilestoneStatus();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone Data")
    FEng_WalkAroundMilestone CurrentMilestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype State")
    EEng_PrototypeState PrototypeState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
    TArray<FEng_PrototypeRequirement> RequirementsList;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    float LastValidationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bAutoValidateOnTick;

private:
    // Internal validation helpers
    bool CheckCharacterInWorld();
    bool CheckLandscapeExists();
    bool CheckDinosaurActors();
    bool CheckLightingSetup();
    
    void InitializeRequirements();
    void UpdateMilestoneFromValidation();
    void LogValidationResults(const FString& ComponentName, bool bIsValid);
};