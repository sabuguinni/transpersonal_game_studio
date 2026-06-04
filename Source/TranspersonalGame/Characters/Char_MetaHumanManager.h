#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Char_MetaHumanManager.generated.h"

class ATranspersonalCharacter;
class USkeletalMesh;
class UMaterialInterface;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_CharacterPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString PresetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EChar_CharacterRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EChar_Gender Gender;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EChar_AgeGroup AgeGroup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString MeshPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString MaterialPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> AccessoryPaths;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FLinearColor SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FLinearColor HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float SurvivalWear;

    FChar_CharacterPreset()
    {
        PresetName = TEXT("Default");
        Role = EChar_CharacterRole::Survivor;
        Gender = EChar_Gender::Male;
        AgeGroup = EChar_AgeGroup::Adult;
        MeshPath = TEXT("");
        MaterialPath = TEXT("");
        SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        HairColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
        SurvivalWear = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_DiversitySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity")
    bool bEnableEthnicDiversity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity")
    bool bEnableAgeDiversity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity")
    bool bEnableGenderBalance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity")
    bool bEnableRoleSpecialization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity")
    float SkinToneVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity")
    float BodyTypeVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity")
    float FacialFeatureVariation;

    FChar_DiversitySettings()
    {
        bEnableEthnicDiversity = true;
        bEnableAgeDiversity = true;
        bEnableGenderBalance = true;
        bEnableRoleSpecialization = true;
        SkinToneVariation = 0.8f;
        BodyTypeVariation = 0.6f;
        FacialFeatureVariation = 0.7f;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_MetaHumanManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_MetaHumanManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman System")
    TArray<FChar_CharacterPreset> CharacterPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman System")
    FChar_DiversitySettings DiversitySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman System")
    TArray<TSoftObjectPtr<USkeletalMesh>> AvailableMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman System")
    TArray<TSoftObjectPtr<UMaterialInterface>> AvailableMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman System")
    int32 MaxNPCsPerRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman System")
    float CharacterSpawnRadius;

public:
    UFUNCTION(BlueprintCallable, Category = "MetaHuman System")
    ATranspersonalCharacter* SpawnCharacterFromPreset(const FChar_CharacterPreset& Preset, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman System")
    FChar_CharacterPreset GenerateRandomPreset(EChar_CharacterRole DesiredRole = EChar_CharacterRole::Survivor);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman System")
    void ApplyPresetToCharacter(ATranspersonalCharacter* Character, const FChar_CharacterPreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman System")
    TArray<ATranspersonalCharacter*> SpawnDiverseNPCGroup(const FVector& CenterLocation, int32 GroupSize = 5);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman System")
    void InitializePresetDatabase();

    UFUNCTION(BlueprintCallable, Category = "MetaHuman System")
    FChar_CharacterPreset GetPresetByName(const FString& PresetName) const;

    UFUNCTION(BlueprintCallable, Category = "MetaHuman System")
    TArray<FChar_CharacterPreset> GetPresetsByRole(EChar_CharacterRole Role) const;

    UFUNCTION(BlueprintCallable, Category = "MetaHuman System", CallInEditor = true)
    void PreviewCharacterPreset(const FChar_CharacterPreset& Preset);

private:
    void CreateDefaultPresets();
    FLinearColor GenerateRandomSkinTone() const;
    FLinearColor GenerateRandomHairColor() const;
    FString SelectRandomMeshPath() const;
    FString SelectRandomMaterialPath() const;
    TArray<FString> SelectRandomAccessories(EChar_CharacterRole Role) const;
};