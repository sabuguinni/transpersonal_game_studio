#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Char_CretaceousTribalManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalAppearanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Appearance")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Appearance")
    EChar_TribalRole TribalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Appearance")
    EChar_SkinTone SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Appearance")
    EChar_BodyType BodyType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Appearance")
    float Age;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Appearance")
    TArray<FString> TribalMarkings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Appearance")
    TArray<FString> ClothingItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Appearance")
    TArray<FString> ToolsAndWeapons;

    FChar_TribalAppearanceData()
    {
        CharacterName = TEXT("Tribal_Character");
        TribalRole = EChar_TribalRole::Hunter;
        SkinTone = EChar_SkinTone::Medium;
        BodyType = EChar_BodyType::Athletic;
        Age = 25.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_CretaceousTribalConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Config")
    int32 MaxTribalNPCs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Config")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Config")
    TArray<FChar_TribalAppearanceData> TribalVariations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Config")
    bool bEnableTribalMarkings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Config")
    bool bEnableWeatheringEffects;

    FChar_CretaceousTribalConfig()
    {
        MaxTribalNPCs = 20;
        SpawnRadius = 10000.0f;
        bEnableTribalMarkings = true;
        bEnableWeatheringEffects = true;
    }
};

UCLASS(ClassGroup=(Character), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_CretaceousTribalManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_CretaceousTribalManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core tribal management functions
    UFUNCTION(BlueprintCallable, Category = "Cretaceous Tribal")
    void InitializeTribalSystem();

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Tribal")
    void SpawnTribalNPC(const FVector& Location, const FChar_TribalAppearanceData& AppearanceData);

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Tribal")
    void ApplyTribalAppearance(class APawn* TargetPawn, const FChar_TribalAppearanceData& AppearanceData);

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Tribal")
    FChar_TribalAppearanceData GenerateRandomTribalAppearance();

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Tribal")
    void UpdateTribalMarkings(class APawn* TargetPawn, const TArray<FString>& Markings);

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Tribal")
    void ApplyWeatheringEffects(class APawn* TargetPawn, float WeatheringIntensity);

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Tribal")
    void SetTribalClothing(class APawn* TargetPawn, const TArray<FString>& ClothingItems);

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Tribal")
    void EquipTribalTools(class APawn* TargetPawn, const TArray<FString>& Tools);

    // Configuration and management
    UFUNCTION(BlueprintCallable, Category = "Cretaceous Tribal")
    void SetTribalConfiguration(const FChar_CretaceousTribalConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Tribal")
    FChar_CretaceousTribalConfig GetTribalConfiguration() const;

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Tribal")
    int32 GetActiveTribalNPCCount() const;

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Tribal")
    TArray<class APawn*> GetAllTribalNPCs() const;

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Tribal")
    void RemoveTribalNPC(class APawn* NPCToRemove);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    FChar_CretaceousTribalConfig TribalConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
    TArray<class APawn*> ActiveTribalNPCs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
    bool bIsInitialized;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
    float LastSpawnTime;

private:
    void ValidateTribalConfiguration();
    FChar_TribalAppearanceData CreateTribalVariation(EChar_TribalRole Role);
    void CleanupInvalidNPCs();
};