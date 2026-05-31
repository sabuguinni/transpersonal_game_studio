#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "Eng_ArchitectureValidator.generated.h"

UENUM(BlueprintType)
enum class EEng_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Warning     UMETA(DisplayName = "Warning"), 
    Error       UMETA(DisplayName = "Error"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FEng_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EEng_ValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float Timestamp;

    FEng_ValidationReport()
    {
        ModuleName = TEXT("");
        Result = EEng_ValidationResult::Pass;
        Message = TEXT("");
        Timestamp = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ArchitectureValidator : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateSystemArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateModuleIntegrity(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FEng_ValidationReport> GetValidationReports() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ClearValidationReports();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateActorCounts();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateBiomeDistribution();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateDinosaurLimits();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FEng_ValidationReport> ValidationReports;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 MaxActorsPerBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 MaxTotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 MaxDinosaurs;

private:
    void AddValidationReport(const FString& ModuleName, EEng_ValidationResult Result, const FString& Message);
    bool ValidateClassExists(const FString& ClassName, const FString& ModuleName);
    bool ValidateComponentRegistration();
    bool ValidateSubsystemInitialization();
};