// TranscendentalLandscape.cpp
// Implementação do sistema de paisagens transcendentais
#include "TranscendentalLandscape.h"
#include "Engine/Engine.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/StaticMesh.h"
#include "Components/InstancedStaticMeshComponent.h"

ATranscendentalLandscape::ATranscendentalLandscape()
{
    PrimaryActorTick.bCanEverTick = true;

    // Criar componentes
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    TerrainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TerrainMesh"));
    TerrainMesh->SetupAttachment(RootComponent);
    
    VegetationInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("VegetationInstances"));
    VegetationInstances->SetupAttachment(RootComponent);
    
    CrystalInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("CrystalInstances"));
    CrystalInstances->SetupAttachment(RootComponent);
    
    EnergyNodeInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("EnergyNodeInstances"));
    EnergyNodeInstances->SetupAttachment(RootComponent);

    // Inicializar variáveis
    CurrentTransitionTime = 0.0f;
    TargetTransitionTime = 2.0f;
    PreviousState = EConsciousnessState::Normal;
    bNeedsRegeneration = true;
}

void ATranscendentalLandscape::BeginPlay()
{
    Super::BeginPlay();
    
    // Gerar paisagem inicial
    GenerateLandscape();
    PopulateVegetation();
    PlaceCrystals();
    CreateEnergyNodes();
}

void ATranscendentalLandscape::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateVisualEffects(DeltaTime);
    ProcessConsciousnessTransition(DeltaTime);
    AnimateGeometry(DeltaTime);
}

void ATranscendentalLandscape::GenerateLandscape()
{
    if (!bNeedsRegeneration) return;
    
    UE_LOG(LogTemp, Warning, TEXT("Generating transcendental landscape..."));
    
    // Gerar heightmap
    GenerateHeightmap();
    
    // Aplicar ruído fractal para complexidade
    ApplyFractalNoise(HeightmapData, NoiseScale, MaxHeight);
    
    // Suavizar terreno
    SmoothTerrain(HeightmapData, 2);
    
    // Criar mesh procedural do terreno
    // Nota: Em produção, usaríamos ProceduralMeshComponent ou Landscape
    
    bNeedsRegeneration = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Landscape generation complete."));
}

void ATranscendentalLandscape::GenerateHeightmap()
{
    int32 TotalPoints = TerrainResolution * TerrainResolution;
    HeightmapData.SetNum(TotalPoints);
    
    for (int32 Y = 0; Y < TerrainResolution; Y++)
    {
        for (int32 X = 0; X < TerrainResolution; X++)
        {
            int32 Index = Y * TerrainResolution + X;
            
            // Coordenadas normalizadas
            float NormX = (float)X / (float)TerrainResolution;
            float NormY = (float)Y / (float)TerrainResolution;
            
            // Ruído base
            float Height = FMath::PerlinNoise2D(FVector2D(NormX * 10.0f, NormY * 10.0f));
            
            // Adicionar variações baseadas no estado de consciência
            switch (CurrentState)
            {
                case EConsciousnessState::Meditative:
                    Height += FMath::Sin(NormX * PI * 4.0f) * FMath::Sin(NormY * PI * 4.0f) * 0.3f;
                    break;
                    
                case EConsciousnessState::Psychedelic:
                    Height += FMath::PerlinNoise2D(FVector2D(NormX * 50.0f, NormY * 50.0f)) * 0.5f;
                    Height += FMath::Sin(NormX * PI * 20.0f) * FMath::Cos(NormY * PI * 20.0f) * 0.2f;
                    break;
                    
                case EConsciousnessState::Mystical:
                    {
                        float CenterX = 0.5f;
                        float CenterY = 0.5f;
                        float Distance = FMath::Sqrt((NormX - CenterX) * (NormX - CenterX) + 
                                                   (NormY - CenterY) * (NormY - CenterY));
                        Height += FMath::Sin(Distance * PI * 10.0f) * FMath::Exp(-Distance * 2.0f) * 0.8f;
                    }
                    break;
                    
                case EConsciousnessState::Transcendent:
                    Height += FMath::PerlinNoise2D(FVector2D(NormX * 100.0f, NormY * 100.0f)) * 0.3f;
                    Height += FMath::Sin(NormX * PI * 8.0f + GetWorld()->GetTimeSeconds()) * 0.2f;
                    break;
                    
                default:
                    break;
            }
            
            HeightmapData[Index] = FMath::Clamp(Height, -1.0f, 1.0f) * MaxHeight;
        }
    }
}

void ATranscendentalLandscape::ApplyFractalNoise(TArray<float>& Heights, float Frequency, float Amplitude)
{
    for (int32 Octave = 0; Octave < NoiseOctaves; Octave++)
    {
        float OctaveFreq = Frequency * FMath::Pow(NoiseLacunarity, Octave);
        float OctaveAmp = Amplitude * FMath::Pow(NoisePersistence, Octave);
        
        for (int32 Y = 0; Y < TerrainResolution; Y++)
        {
            for (int32 X = 0; X < TerrainResolution; X++)
            {
                int32 Index = Y * TerrainResolution + X;
                
                float NormX = (float)X / (float)TerrainResolution;
                float NormY = (float)Y / (float)TerrainResolution;
                
                float NoiseValue = FMath::PerlinNoise2D(FVector2D(NormX / OctaveFreq, NormY / OctaveFreq));
                Heights[Index] += NoiseValue * OctaveAmp;
            }
        }
    }
}

void ATranscendentalLandscape::SmoothTerrain(TArray<float>& Heights, int32 Iterations)
{
    for (int32 Iter = 0; Iter < Iterations; Iter++)
    {
        TArray<float> SmoothedHeights = Heights;
        
        for (int32 Y = 1; Y < TerrainResolution - 1; Y++)
        {
            for (int32 X = 1; X < TerrainResolution - 1; X++)
            {
                int32 Index = Y * TerrainResolution + X;
                
                float Sum = 0.0f;
                int32 Count = 0;
                
                // Média dos vizinhos
                for (int32 DY = -1; DY <= 1; DY++)
                {
                    for (int32 DX = -1; DX <= 1; DX++)
                    {
                        int32 NeighborIndex = (Y + DY) * TerrainResolution + (X + DX);
                        Sum += Heights[NeighborIndex];
                        Count++;
                    }
                }
                
                SmoothedHeights[Index] = Sum / Count;
            }
        }
        
        Heights = SmoothedHeights;
    }
}

void ATranscendentalLandscape::PopulateVegetation()
{
    if (VegetationLayers.Num() == 0) return;
    
    VegetationInstances->ClearInstances();
    
    for (const FLandscapeLayer& Layer : VegetationLayers)
    {
        if (!Layer.Mesh) continue;
        
        VegetationInstances->SetStaticMesh(Layer.Mesh);
        if (Layer.Material)
        {
            VegetationInstances->SetMaterial(0, Layer.Material);
        }
        
        int32 InstanceCount = FMath::RoundToInt(LandscapeSize.X * LandscapeSize.Y * Layer.Density * 0.0001f);
        PlaceInstancesOnTerrain(VegetationInstances, Layer, InstanceCount);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Vegetation populated: %d instances"), VegetationInstances->GetInstanceCount());
}

void ATranscendentalLandscape::PlaceCrystals()
{
    if (CrystalLayers.Num() == 0) return;
    
    CrystalInstances->ClearInstances();
    
    for (const FLandscapeLayer& Layer : CrystalLayers)
    {
        if (!Layer.Mesh) continue;
        
        CrystalInstances->SetStaticMesh(Layer.Mesh);
        if (Layer.Material)
        {
            CrystalInstances->SetMaterial(0, Layer.Material);
        }
        
        int32 InstanceCount = FMath::RoundToInt(LandscapeSize.X * LandscapeSize.Y * Layer.Density * 0.00005f);
        PlaceInstancesOnTerrain(CrystalInstances, Layer, InstanceCount);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crystals placed: %d instances"), CrystalInstances->GetInstanceCount());
}

void ATranscendentalLandscape::CreateEnergyNodes()
{
    EnergyNodeInstances->ClearInstances();
    
    // Criar nós de energia em padrão geométrico sagrado
    int32 NodeCount = 12; // Baseado em geometria sagrada
    float Radius = FMath::Min(LandscapeSize.X, LandscapeSize.Y) * 0.3f;
    
    for (int32 i = 0; i < NodeCount; i++)
    {
        float Angle = (float)i / (float)NodeCount * PI * 2.0f;
        float X = FMath::Cos(Angle) * Radius;
        float Y = FMath::Sin(Angle) * Radius;
        float Z = GetHeightAtLocation(FVector2D(X, Y)) + 100.0f;
        
        FVector Location(X, Y, Z);
        FRotator Rotation = FRotator::ZeroRotator;
        FVector Scale = FVector::OneVector;
        
        FTransform Transform(Rotation, Location, Scale);
        EnergyNodeInstances->AddInstance(Transform);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Energy nodes created: %d"), NodeCount);
}

void ATranscendentalLandscape::PlaceInstancesOnTerrain(UInstancedStaticMeshComponent* InstanceComponent, 
                                                      const FLandscapeLayer& Layer, int32 Count)
{
    for (int32 i = 0; i < Count; i++)
    {
        // Posição aleatória na paisagem
        float X = FMath::RandRange(-LandscapeSize.X * 0.5f, LandscapeSize.X * 0.5f);
        float Y = FMath::RandRange(-LandscapeSize.Y * 0.5f, LandscapeSize.Y * 0.5f);
        float Z = GetHeightAtLocation(FVector2D(X, Y));
        
        FVector Location(X, Y, Z);
        
        // Verificar se é um local válido
        if (!IsValidPlacement(Location, Layer)) continue;
        
        // Rotação aleatória
        FRotator Rotation(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
        
        // Escala aleatória dentro do range
        float Scale = FMath::RandRange(Layer.ScaleRange.X, Layer.ScaleRange.Y);
        FVector ScaleVector(Scale, Scale, Scale);
        
        // Adicionar instância
        FTransform Transform(Rotation, Location, ScaleVector);
        InstanceComponent->AddInstance(Transform);
    }
}

bool ATranscendentalLandscape::IsValidPlacement(const FVector& Location, const FLandscapeLayer& Layer)
{
    float Height = Location.Z;
    if (Height < Layer.HeightMin || Height > Layer.HeightMax) return false;
    
    float Slope = GetSlopeAtLocation(FVector2D(Location.X, Location.Y));
    if (Slope < Layer.SlopeMin || Slope > Layer.SlopMax) return false;
    
    return true;
}

float ATranscendentalLandscape::GetHeightAtLocation(const FVector2D& Location)
{
    // Converter coordenadas do mundo para coordenadas do heightmap
    float NormX = (Location.X + LandscapeSize.X * 0.5f) / LandscapeSize.X;
    float NormY = (Location.Y + LandscapeSize.Y * 0.5f) / LandscapeSize.Y;
    
    NormX = FMath::Clamp(NormX, 0.0f, 1.0f);
    NormY = FMath::Clamp(NormY, 0.0f, 1.0f);
    
    int32 X = FMath::FloorToInt(NormX * (TerrainResolution - 1));
    int32 Y = FMath::FloorToInt(NormY * (TerrainResolution - 1));
    
    int32 Index = Y * TerrainResolution + X;
    
    if (HeightmapData.IsValidIndex(Index))
    {
        return HeightmapData[Index];
    }
    
    return 0.0f;
}

float ATranscendentalLandscape::GetSlopeAtLocation(const FVector2D& Location)
{
    float SampleDistance = 10.0f;
    
    float HeightCenter = GetHeightAtLocation(Location);
    float HeightRight = GetHeightAtLocation(Location + FVector2D(SampleDistance, 0.0f));
    float HeightUp = GetHeightAtLocation(Location + FVector2D(0.0f, SampleDistance));
    
    float SlopeX = FMath::Abs(HeightRight - HeightCenter) / SampleDistance;
    float SlopeY = FMath::Abs(HeightUp - HeightCenter) / SampleDistance;
    
    float Slope = FMath::Sqrt(SlopeX * SlopeX + SlopeY * SlopeY);
    return FMath::RadiansToDegrees(FMath::Atan(Slope));
}

void ATranscendentalLandscape::TransitionToState(EConsciousnessState NewState)
{
    if (NewState == CurrentState) return;
    
    PreviousState = CurrentState;
    CurrentState = NewState;
    CurrentTransitionTime = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Transitioning to consciousness state: %d"), (int32)NewState);
    
    // Regenerar paisagem para o novo estado
    bNeedsRegeneration = true;
    GenerateLandscape();
}

void ATranscendentalLandscape::UpdateVisualEffects(float DeltaTime)
{
    UpdateMaterialParameters();
    
    // Pulsar nós de energia
    float PulseIntensity = FMath::Sin(GetWorld()->GetTimeSeconds() * 2.0f) * 0.5f + 0.5f;
    PulseEnergyField(PulseIntensity * EnergyFlowIntensity);
}

void ATranscendentalLandscape::UpdateMaterialParameters()
{
    if (!EnvironmentParameters) return;
    
    UMaterialParameterCollectionInstance* Instance = GetWorld()->GetParameterCollectionInstance(EnvironmentParameters);
    if (!Instance) return;
    
    // Atualizar parâmetros baseados no estado de consciência
    Instance->SetScalarParameterValue(FName("ConsciousnessLevel"), (float)CurrentState);
    Instance->SetScalarParameterValue(FName("EnergyFlow"), EnergyFlowIntensity);
    Instance->SetVectorParameterValue(FName("AuraColor"), FVector(AuraColor.R, AuraColor.G, AuraColor.B));
    Instance->SetScalarParameterValue(FName("GeometryMorph"), GeometryMorphing);
    Instance->SetScalarParameterValue(FName("FractalComplexity"), FractalComplexity);
    Instance->SetScalarParameterValue(FName("TimeFlow"), GetWorld()->GetTimeSeconds());
}

void ATranscendentalLandscape::AnimateGeometry(float DeltaTime)
{
    // Animar morfismo geométrico baseado no estado de consciência
    float TargetMorphing = 0.0f;
    
    switch (CurrentState)
    {
        case EConsciousnessState::Psychedelic:
            TargetMorphing = 0.3f;
            break;
        case EConsciousnessState::Mystical:
            TargetMorphing = 0.5f;
            break;
        case EConsciousnessState::Transcendent:
            TargetMorphing = 0.8f;
            break;
        default:
            TargetMorphing = 0.0f;
            break;
    }
    
    GeometryMorphing = FMath::FInterpTo(GeometryMorphing, TargetMorphing, DeltaTime, 2.0f);
}

void ATranscendentalLandscape::ProcessConsciousnessTransition(float DeltaTime)
{
    if (CurrentTransitionTime < TargetTransitionTime)
    {
        CurrentTransitionTime += DeltaTime;
        float Alpha = CurrentTransitionTime / TargetTransitionTime;
        
        // Interpolar efeitos visuais durante transição
        float IntensityMultiplier = FMath::Sin(Alpha * PI);
        EnergyFlowIntensity = 1.0f + IntensityMultiplier * 2.0f;
        
        // Atualizar complexidade fractal
        FractalComplexity = 1.0f + Alpha * (float)CurrentState * 0.5f;
    }
}

void ATranscendentalLandscape::PulseEnergyField(float Intensity)
{
    // Atualizar transformações dos nós de energia
    for (int32 i = 0; i < EnergyNodeInstances->GetInstanceCount(); i++)
    {
        FTransform Transform;
        EnergyNodeInstances->GetInstanceTransform(i, Transform, true);
        
        // Aplicar pulsação
        float PulseScale = 1.0f + Intensity * 0.2f;
        Transform.SetScale3D(FVector(PulseScale));
        
        // Rotação suave
        FRotator CurrentRotation = Transform.GetRotation().Rotator();
        CurrentRotation.Yaw += 30.0f * GetWorld()->GetDeltaSeconds();
        Transform.SetRotation(CurrentRotation.Quaternion());
        
        EnergyNodeInstances->UpdateInstanceTransform(i, Transform, true, true);
    }
}