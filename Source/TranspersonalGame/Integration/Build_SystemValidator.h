#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Build_IntegrationReport.h"
#include "Build_SystemValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_SystemType : uint8
{
    Core        UMETA(DisplayName = "Core Systems"),
    World       UMETA(DisplayName = "World Generation"),
    Character   UMETA(DisplayName = "Character Systems"),
    AI          UMETA(DisplayName = "AI Systems"),
    Audio       UMETA(DisplayName = "Audio Systems"),
    VFX         UMETA(DisplayName = "VFX Systems"),
    UI          UMETA(DisplayName = "UI Systems"),
    Network     UMETA(DisplayName = "Network Systems")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    EBuild_SystemType SystemType;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    TArray<FString> RequiredClasses;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    TArray<FString> OptionalClasses;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    bool bIsEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    bool bIsValidated;

    FBuild_SystemInfo()
    {
        SystemName = TEXT("");
        SystemType = EBuild_SystemType::Core;
        bIsEnabled = true;
        bIsValidated = false;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuild_SystemValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_SystemValidator();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FBuild_SystemInfo> RegisteredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    UBuild_IntegrationReport* CurrentReport;

    UFUNCTION(BlueprintCallable, Category = "Validation")
    void RegisterSystem(const FBuild_SystemInfo& SystemInfo);

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    UBuild_IntegrationReport* GenerateFullReport();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    TArray<FString> GetFailedSystems();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    TArray<FString> GetMissingDependencies();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool IsSystemRegistered(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Validation")
    FBuild_SystemInfo GetSystemInfo(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Validation")
    void EnableSystem(const FString& SystemName, bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Validation")
    int32 GetValidatedSystemCount();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    float GetSystemValidationProgress();

private:
    void RegisterCoreSystem();
    void RegisterWorldSystems();
    void RegisterCharacterSystems();
    void RegisterAISystems();
    
    bool ValidateSystemClasses(const FBuild_SystemInfo& SystemInfo);
    bool ValidateSystemDependencies(const FBuild_SystemInfo& SystemInfo);
    bool CheckClassExists(const FString& ClassName);
    
    TArray<FString> FailedValidations;
    TArray<FString> MissingDependencies;
};