#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Materials/MaterialInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "SharedTypes.h"
#include "CharacterCustomizationManager.generated.h"

class USkeletalMesh;
class UMaterialInstance;
class UTexture2D;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalAppearanceData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<USkeletalMesh> BaseMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<UMaterialInstance> SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<UMaterialInstance> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor ClothingColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<FString> TribalMarkings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float ScarIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float WeatheringLevel;

    FChar_TribalAppearanceData()
    {
        CharacterName = TEXT("Unnamed Survivor");
        SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);
        ClothingColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
        ScarIntensity = 0.3f;
        WeatheringLevel = 0.7f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_EmotionState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Fear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Determination;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Exhaustion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Pain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Hope;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Anger;

    FChar_EmotionState()
    {
        Fear = 0.0f;
        Determination = 0.5f;
        Exhaustion = 0.2f;
        Pain = 0.0f;
        Hope = 0.6f;
        Anger = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCharacterCustomizationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCharacterCustomizationManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Character appearance customization
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyTribalAppearance(USkeletalMeshComponent* MeshComponent, const FChar_TribalAppearanceData& AppearanceData);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    FChar_TribalAppearanceData GenerateRandomTribalAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetCharacterSkinTone(USkeletalMeshComponent* MeshComponent, const FLinearColor& SkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetCharacterClothing(USkeletalMeshComponent* MeshComponent, const FLinearColor& ClothingColor, float WeatheringLevel);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void AddTribalMarkings(USkeletalMeshComponent* MeshComponent, const TArray<FString>& Markings);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetScarring(USkeletalMeshComponent* MeshComponent, float ScarIntensity);

    // Emotion and expression system
    UFUNCTION(BlueprintCallable, Category = "Character Expression")
    void UpdateEmotionState(USkeletalMeshComponent* MeshComponent, const FChar_EmotionState& EmotionState);

    UFUNCTION(BlueprintCallable, Category = "Character Expression")
    void SetFacialExpression(USkeletalMeshComponent* MeshComponent, const FString& ExpressionName, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Character Expression")
    void PlayEmotionalReaction(USkeletalMeshComponent* MeshComponent, EChar_SurvivalEmotion Emotion, float Duration);

    // Character diversity management
    UFUNCTION(BlueprintCallable, Category = "Character Diversity")
    TArray<FChar_TribalAppearanceData> GenerateTribalGroup(int32 GroupSize, bool bEnsureDiversity = true);

    UFUNCTION(BlueprintCallable, Category = "Character Diversity")
    void ValidateCharacterDiversity(const TArray<FChar_TribalAppearanceData>& Characters, bool& bIsValid, FString& ValidationMessage);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data Tables")
    TSoftObjectPtr<UDataTable> TribalAppearanceTable;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInstance>> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInstance>> ClothingMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Meshes")
    TArray<TSoftObjectPtr<USkeletalMesh>> TribalMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Expressions")
    TMap<FString, float> DefaultExpressionWeights;

private:
    void InitializeAppearanceData();
    void LoadTribalAssets();
    FLinearColor GenerateRandomSkinTone();
    FLinearColor GenerateRandomHairColor();
    TArray<FString> GenerateRandomTribalMarkings();
    
    bool bIsInitialized;
    TArray<FChar_TribalAppearanceData> PresetAppearances;
};