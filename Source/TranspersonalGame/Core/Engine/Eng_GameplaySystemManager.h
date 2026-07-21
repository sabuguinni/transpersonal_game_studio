#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_GameplaySystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_GameplaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    float PlayerMovementSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    float PlayerJumpVelocity = 420.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    float PlayerStaminaDrainRate = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    float PlayerHealthRegenRate = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bEnablePvP = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    int32 MaxPlayersPerServer = 32;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_DinosaurSpawnSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaurs")
    int32 MaxDinosaursPerBiome = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaurs")
    float DinosaurSpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaurs")
    float DinosaurRespawnTime = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaurs")
    bool bEnableDinosaurMigration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaurs")
    float DinosaurAggressionMultiplier = 1.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_GameplaySystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Gameplay System")
    void InitializeGameplaySettings();

    UFUNCTION(BlueprintCallable, Category = "Gameplay System")
    void ApplyGameplaySettings(const FEng_GameplaySettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Gameplay System")
    FEng_GameplaySettings GetGameplaySettings() const { return GameplaySettings; }

    UFUNCTION(BlueprintCallable, Category = "Gameplay System")
    void ConfigureDinosaurSpawning(const FEng_DinosaurSpawnSettings& SpawnSettings);

    UFUNCTION(BlueprintCallable, Category = "Gameplay System")
    FEng_DinosaurSpawnSettings GetDinosaurSpawnSettings() const { return DinosaurSpawnSettings; }

    UFUNCTION(BlueprintCallable, Category = "Gameplay System")
    void SetDifficultyLevel(EDifficultyLevel NewDifficulty);

    UFUNCTION(BlueprintCallable, Category = "Gameplay System")
    EDifficultyLevel GetDifficultyLevel() const { return CurrentDifficulty; }

    UFUNCTION(BlueprintCallable, Category = "Gameplay System")
    void EnableDebugMode(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Gameplay System")
    bool IsDebugModeEnabled() const { return bDebugMode; }

    UFUNCTION(BlueprintCallable, Category = "Gameplay System")
    void ValidateGameplayIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Gameplay System")
    TArray<FString> GetSystemStatus();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Gameplay System")
    FEng_GameplaySettings GameplaySettings;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay System")
    FEng_DinosaurSpawnSettings DinosaurSpawnSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay System")
    EDifficultyLevel CurrentDifficulty = EDifficultyLevel::Medium;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay System")
    bool bDebugMode = false;

    UPROPERTY(BlueprintReadOnly, Category = "Gameplay System")
    bool bSystemInitialized = false;

private:
    void LoadDefaultSettings();
    void SaveSettingsToConfig();
    void LoadSettingsFromConfig();
    void BroadcastSettingsChanged();
};