#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_ArchitecturalFramework.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleDependency
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Architecture")
    FString ModuleName;

    UPROPERTY(BlueprintReadWrite, Category = "Architecture")
    FString DependsOn;

    UPROPERTY(BlueprintReadWrite, Category = "Architecture")
    bool bIsRequired;

    UPROPERTY(BlueprintReadWrite, Category = "Architecture")
    float LoadPriority;

    FEng_ModuleDependency()
        : ModuleName(TEXT(""))
        , DependsOn(TEXT(""))
        , bIsRequired(true)
        , LoadPriority(1.0f)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemArchitecture
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Architecture")
    TArray<FEng_ModuleDependency> Dependencies;

    UPROPERTY(BlueprintReadWrite, Category = "Architecture")
    TMap<FString, float> SystemPriorities;

    UPROPERTY(BlueprintReadWrite, Category = "Architecture")
    TArray<FString> InitializationOrder;

    UPROPERTY(BlueprintReadWrite, Category = "Architecture")
    bool bValidated;

    FEng_SystemArchitecture()
        : bValidated(false)
    {}
};

UENUM(BlueprintType)
enum class EEng_ArchitecturalLayer : uint8
{
    Core        UMETA(DisplayName = "Core Systems"),
    World       UMETA(DisplayName = "World Generation"),
    Gameplay    UMETA(DisplayName = "Gameplay Systems"),
    AI          UMETA(DisplayName = "AI & Behavior"),
    Audio       UMETA(DisplayName = "Audio Systems"),
    VFX         UMETA(DisplayName = "Visual Effects"),
    UI          UMETA(DisplayName = "User Interface")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ArchitecturalFramework : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_ArchitecturalFramework();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterModule(const FString& ModuleName, EEng_ArchitecturalLayer Layer, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void AddModuleDependency(const FString& ModuleName, const FString& DependsOn, bool bRequired = true);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FString> GetInitializationOrder() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsModuleRegistered(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float GetModulePriority(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor = true)
    void DebugPrintArchitecture();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    FEng_SystemArchitecture SystemArchitecture;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    TMap<FString, EEng_ArchitecturalLayer> ModuleLayers;

    UPROPERTY(BlueprintReadOnly, Category = "Architecture")
    bool bInitialized;

private:
    void BuildInitializationOrder();
    bool ValidateDependencies();
    void RegisterCoreModules();
};