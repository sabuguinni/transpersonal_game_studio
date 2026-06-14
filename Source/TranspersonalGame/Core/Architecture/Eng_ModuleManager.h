#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Eng_ModuleManager.generated.h"

UENUM(BlueprintType)
enum class EEng_ModuleState : uint8
{
    Unloaded = 0,
    Loading = 1,
    Loaded = 2,
    Initializing = 3,
    Ready = 4,
    Error = 5
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly)
    EEng_ModuleState State;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly)
    float LoadTime;

    UPROPERTY(BlueprintReadOnly)
    FString ErrorMessage;

    FEng_ModuleInfo()
        : State(EEng_ModuleState::Unloaded)
        , LoadTime(0.0f)
    {}
};

UCLASS()
class TRANSPERSONALGAME_API UEng_ModuleManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    void RegisterModule(const FString& ModuleName, const TArray<FString>& Dependencies = TArray<FString>());

    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    void LoadModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    void UnloadModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    bool IsModuleLoaded(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    EEng_ModuleState GetModuleState(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    TArray<FEng_ModuleInfo> GetAllModules() const;

    UFUNCTION(BlueprintCallable, Category = "Module Manager")
    void ValidateModuleDependencies();

private:
    UPROPERTY()
    TMap<FString, FEng_ModuleInfo> Modules;

    void SetModuleState(const FString& ModuleName, EEng_ModuleState NewState, const FString& ErrorMsg = TEXT(""));
    bool CheckDependencies(const FString& ModuleName) const;
    void LogModuleStatus();
};

#include "Eng_ModuleManager.generated.h"