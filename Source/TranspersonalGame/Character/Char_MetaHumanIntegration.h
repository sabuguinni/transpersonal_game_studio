#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Char_MetaHumanIntegration.generated.h"

UENUM(BlueprintType)
enum class EChar_MetaHumanPreset : uint8
{
    TribalWarrior   UMETA(DisplayName = "Tribal Warrior"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Gatherer        UMETA(DisplayName = "Gatherer"),
    Elder           UMETA(DisplayName = "Elder"),
    Child           UMETA(DisplayName = "Child"),
    Shaman          UMETA(DisplayName = "Shaman")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_MetaHumanData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    FString MetaHumanID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    EChar_MetaHumanPreset PresetType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<class USkeletalMesh> BodyMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<class USkeletalMesh> FaceMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<class UAnimBlueprint> AnimBlueprint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TArray<TSoftObjectPtr<class UMaterialInterface>> Materials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    float Height;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    float Weight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    bool bIsMale;

    FChar_MetaHumanData()
    {
        MetaHumanID = TEXT("");
        DisplayName = TEXT("Unknown");
        PresetType = EChar_MetaHumanPreset::TribalWarrior;
        Height = 175.0f;
        Weight = 70.0f;
        bIsMale = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_MetaHumanIntegration : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_MetaHumanIntegration();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<class UDataTable> MetaHumanDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    FChar_MetaHumanData CurrentMetaHumanData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MetaHuman")
    class USkeletalMeshComponent* BodyMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MetaHuman")
    class USkeletalMeshComponent* FaceMeshComponent;

public:
    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    bool LoadMetaHumanPreset(EChar_MetaHumanPreset PresetType);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    bool LoadMetaHumanByID(const FString& MetaHumanID);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void ApplyMetaHumanData(const FChar_MetaHumanData& MetaHumanData);

    UFUNCTION(BlueprintPure, Category = "MetaHuman")
    FChar_MetaHumanData GetCurrentMetaHumanData() const { return CurrentMetaHumanData; }

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    TArray<FChar_MetaHumanData> GetAvailableMetaHumans() const;

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void SetBodyScale(float ScaleFactor);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void SetAnimationBlueprint(TSoftObjectPtr<class UAnimBlueprint> AnimBP);

private:
    void InitializeMeshComponents();
    void LoadMeshAssets();
    void ApplyMaterials();
    FChar_MetaHumanData* FindMetaHumanData(EChar_MetaHumanPreset PresetType) const;
    FChar_MetaHumanData* FindMetaHumanDataByID(const FString& MetaHumanID) const;
};