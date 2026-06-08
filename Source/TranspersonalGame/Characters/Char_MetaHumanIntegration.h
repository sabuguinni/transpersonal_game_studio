#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Char_MetaHumanIntegration.generated.h"

UENUM(BlueprintType)
enum class EChar_MetaHumanType : uint8
{
    TribalWarrior     UMETA(DisplayName = "Tribal Warrior"),
    TribalHunter      UMETA(DisplayName = "Tribal Hunter"),
    TribalShaman      UMETA(DisplayName = "Tribal Shaman"),
    TribalGatherer    UMETA(DisplayName = "Tribal Gatherer"),
    TribalChild       UMETA(DisplayName = "Tribal Child"),
    TribalElder       UMETA(DisplayName = "Tribal Elder")
};

UENUM(BlueprintType)
enum class EChar_BodyBuild : uint8
{
    Lean        UMETA(DisplayName = "Lean"),
    Athletic    UMETA(DisplayName = "Athletic"),
    Muscular    UMETA(DisplayName = "Muscular"),
    Stocky      UMETA(DisplayName = "Stocky"),
    Frail       UMETA(DisplayName = "Frail")
};

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Fair        UMETA(DisplayName = "Fair"),
    Medium      UMETA(DisplayName = "Medium"),
    Olive       UMETA(DisplayName = "Olive"),
    Dark        UMETA(DisplayName = "Dark"),
    Deep        UMETA(DisplayName = "Deep")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_MetaHumanConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EChar_MetaHumanType CharacterType = EChar_MetaHumanType::TribalWarrior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyBuild BodyBuild = EChar_BodyBuild::Athletic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString CharacterName = TEXT("Tribal Character");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TSoftObjectPtr<USkeletalMesh> MetaHumanMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TArray<TSoftObjectPtr<UMaterialInterface>> CharacterMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    bool bHasTribalScars = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    bool bHasTribalTattoos = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    bool bHasBattleScars = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    float AgeMultiplier = 1.0f;

    FChar_MetaHumanConfig()
    {
        CharacterType = EChar_MetaHumanType::TribalWarrior;
        BodyBuild = EChar_BodyBuild::Athletic;
        SkinTone = EChar_SkinTone::Medium;
        bHasTribalScars = true;
        bHasTribalTattoos = false;
        bHasBattleScars = false;
        AgeMultiplier = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_MetaHumanIntegration : public AActor
{
    GENERATED_BODY()

public:
    AChar_MetaHumanIntegration();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class USkeletalMeshComponent* CharacterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class UCapsuleComponent* CapsuleComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Configuration")
    FChar_MetaHumanConfig MetaHumanConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Library")
    TArray<TSoftObjectPtr<USkeletalMesh>> WarriorMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Library")
    TArray<TSoftObjectPtr<USkeletalMesh>> HunterMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Library")
    TArray<TSoftObjectPtr<USkeletalMesh>> ShamanMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Library")
    TArray<TSoftObjectPtr<USkeletalMesh>> GathererMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Library")
    TArray<TSoftObjectPtr<UMaterialInterface>> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Library")
    TArray<TSoftObjectPtr<UMaterialInterface>> ClothingMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Library")
    TArray<TSoftObjectPtr<UMaterialInterface>> TribalMarkingMaterials;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Character Setup")
    void ApplyMetaHumanConfiguration(const FChar_MetaHumanConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Character Setup")
    void SetCharacterType(EChar_MetaHumanType NewType);

    UFUNCTION(BlueprintCallable, Category = "Character Setup")
    void SetBodyBuild(EChar_BodyBuild NewBuild);

    UFUNCTION(BlueprintCallable, Category = "Character Setup")
    void SetSkinTone(EChar_SkinTone NewTone);

    UFUNCTION(BlueprintCallable, Category = "Character Setup")
    void ApplyTribalCustomization();

    UFUNCTION(BlueprintCallable, Category = "Character Setup")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Asset Management")
    void LoadMetaHumanAssets();

    UFUNCTION(BlueprintCallable, Category = "Asset Management")
    USkeletalMesh* GetMeshForCharacterType(EChar_MetaHumanType Type) const;

    UFUNCTION(BlueprintCallable, Category = "Asset Management")
    UMaterialInterface* GetMaterialForSkinTone(EChar_SkinTone Tone) const;

    UFUNCTION(BlueprintCallable, Category = "Validation", CallInEditor)
    void ValidateCharacterSetup();

    UFUNCTION(BlueprintPure, Category = "Character Info")
    FString GetCharacterDescription() const;

    UFUNCTION(BlueprintPure, Category = "Character Info")
    bool IsCharacterValid() const;

protected:
    UFUNCTION()
    void OnMetaHumanAssetLoaded();

    void ApplyMeshToCharacter(USkeletalMesh* NewMesh);
    void ApplyMaterialsToCharacter();
    void SetupCharacterCollision();
    void ConfigureCharacterForType();

private:
    bool bIsInitialized = false;
    bool bAssetsLoaded = false;
    
    void InitializeDefaultAssets();
    void LoadCharacterAssets();
    USkeletalMesh* FindBestAvailableMesh() const;
    UMaterialInterface* FindBestAvailableMaterial() const;
};