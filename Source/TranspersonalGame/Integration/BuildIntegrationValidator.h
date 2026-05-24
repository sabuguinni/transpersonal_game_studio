#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "BuildIntegrationValidator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    bool bIsValid = false;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    int32 LoadedClasses = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    int32 TotalClasses = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    TArray<FString> FailedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    FString ValidationMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Build Validation")
    float ValidationTime = 0.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Module Status")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Module Status")
    bool bIsLoaded = false;

    UPROPERTY(BlueprintReadOnly, Category = "Module Status")
    int32 ClassCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Module Status")
    TArray<FString> LoadedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Module Status")
    FString LastError;
};

/**
 * Build Integration Validator - Validates module loading, class registration, and system integration
 * Ensures all C++ classes are properly compiled and accessible to the UE5 reflection system
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildIntegrationValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildIntegrationValidator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Validates all core game modules and classes
     * @return Validation result with detailed status
     */
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_ValidationResult ValidateAllModules();

    /**
     * Validates a specific module by name
     * @param ModuleName Name of the module to validate
     * @return Module status with loading information
     */
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_ModuleStatus ValidateModule(const FString& ModuleName);

    /**
     * Validates core game classes are properly loaded
     * @return True if all core classes are accessible
     */
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool ValidateCoreClasses();

    /**
     * Validates world state and actor spawning capabilities
     * @param World World context for validation
     * @return True if world state is valid for gameplay
     */
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool ValidateWorldState(UWorld* World);

    /**
     * Runs comprehensive integration tests
     * @return Detailed validation result
     */
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_ValidationResult RunIntegrationTests();

    /**
     * Gets the last validation result
     * @return Cached validation result
     */
    UFUNCTION(BlueprintPure, Category = "Build Validation")
    FBuild_ValidationResult GetLastValidationResult() const { return LastValidationResult; }

    /**
     * Checks if the build is in a valid state for gameplay
     * @return True if build is ready for gameplay testing
     */
    UFUNCTION(BlueprintPure, Category = "Build Validation")
    bool IsBuildValid() const { return LastValidationResult.bIsValid; }

protected:
    /**
     * Validates a single class by path
     * @param ClassPath Full path to the class (e.g., "/Script/TranspersonalGame.TranspersonalCharacter")
     * @return True if class loads successfully
     */
    bool ValidateClass(const FString& ClassPath);

    /**
     * Gets list of core classes that must be validated
     * @return Array of class paths for validation
     */
    TArray<FString> GetCoreClassPaths() const;

    /**
     * Logs validation results to console and file
     * @param Result Validation result to log
     */
    void LogValidationResult(const FBuild_ValidationResult& Result);

private:
    /** Cached last validation result */
    UPROPERTY()
    FBuild_ValidationResult LastValidationResult;

    /** Module status cache */
    UPROPERTY()
    TArray<FBuild_ModuleStatus> ModuleStatusCache;

    /** Validation start time for performance tracking */
    double ValidationStartTime = 0.0;

    /** Core class paths for validation */
    TArray<FString> CoreClassPaths;
};